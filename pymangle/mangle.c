#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mangle.h"
#include "polygon.h"
#include "defs.h"



struct MangleMask* mangle_new()
{
    struct MangleMask* self=NULL;

    self = calloc(1, sizeof(struct MangleMask));
    if (self == NULL) {
        wlog("Failed to allocate MangleMask\n");
        return NULL;
    }

    mangle_clear(self);
    return self;
}



struct MangleMask* mangle_free(struct MangleMask* self)
{
    mangle_clear(self);
    free(self);
    return NULL;
}
void mangle_clear(struct MangleMask* self)
{
    if (self != NULL) {

        free(self->filename);
        self->filename=NULL;

        memset(self->weightfile, 0, sizeof(self->weightfile));

        self->poly_vec = polyvec_free(self->poly_vec);
        self->pixel_list_vec = PixelListVec_free(self->pixel_list_vec);

        if (self->fptr != NULL) {
            fclose(self->fptr);
        }

        self->pixelres=-1;
        self->maxpix=-1;
        self->pixeltype='u';
    }
}
void mangle_set_verbosity(struct MangleMask* self, int verbosity)
{
    if (self) {
        self->verbose=verbosity;
    }
}

void mangle_print(FILE* fptr, struct MangleMask* self, int verbosity)
{
    if (!self || verbosity == 0)
        return;

    size_t npoly;
    size_t npix;

    npoly = (self->poly_vec != NULL) ? self->poly_vec->size : 0;
    npix = (self->pixel_list_vec != NULL) ? self->pixel_list_vec->size : 0;

    fprintf(fptr,
            "Mangle\n"
            "\tfile:       %s\n"
            "\tarea:       %Lg sqdeg\n"
            "\tnpoly:      %ld\n"
            "\tpixeltype:  '%c'\n"
            "\tpixelres:   %ld\n"
            "\tnpix:       %ld\n"
            "\tsnapped:    %d\n"
            "\tbalkanized: %d\n"
	    "\tweightfile: %s\n"
            "\tverbose:    %d\n", 
            self->filename, self->total_area*R2D*R2D, 
            npoly, self->pixeltype, self->pixelres, npix, 
            self->snapped, self->balkanized, self->weightfile,
            self->verbose);

    if (verbosity > 1) {
        print_polygons(fptr,self->poly_vec);
    }
}


int mangle_read(struct MangleMask* self, const char* filename)
{
    int status=1;

    mangle_clear(self);
    self->filename=strdup(filename);
    self->fptr = fopen(filename,"r");
    self->real = 10;  // default
    if (self->fptr == NULL) {
        wlog("Failed to open file for reading: %s\n",filename);
        status=0;
        goto _mangle_read_bail;
    }

    if (!mangle_read_header(self)) {
        status=0;
        goto _mangle_read_bail;
    }

    if (self->verbose)
        wlog("reading %ld polygons\n", self->npoly);
    self->poly_vec = read_polygons(self->fptr, self->npoly);
    if (!self->poly_vec) {
        status=0;
        goto _mangle_read_bail;
    }

    mangle_calc_area_and_maxpix(self);

    if (!set_pixel_map(self)) {
        status=0;
        goto _mangle_read_bail;
    }

_mangle_read_bail:
    return status;
}

// npoly
// snapped
// balkanized
// pixelres
// pixeltype
int mangle_read_header(struct MangleMask* self)
{
    int status=1;

    if (2 != fscanf(self->fptr,"%ld %s", &self->npoly, self->buff)) {
        status = 0;
        wlog("Could not read number of polygons");
        goto _read_header_bail;
    }
    if (0 != strcmp(self->buff,"polygons")) {
        status = 0;
        wlog("Expected keyword 'polygons' but got '%s'", self->buff);
        goto _read_header_bail;
    }

    if (self->verbose)
        wlog("Expect %ld polygons\n", self->npoly);

    // get some metadata
    if (1 != fscanf(self->fptr,"%s", self->buff) ) {
        status=0;
        wlog("Error reading header keyword");
        goto _read_header_bail;
    }
    while (0 != strcmp(self->buff,"polygon")) {
        if (0 == strcmp(self->buff,"snapped")) {
            if (self->verbose) 
                wlog("\tpolygons are snapped\n");
            self->snapped=1;
        } else if (0 == strcmp(self->buff,"balkanized")) {
            if (self->verbose) 
                wlog("\tpolygons are balkanized\n");
            self->balkanized=1;
	} else if (0 == strcmp(self->buff,"real")) {
	    if (1 != fscanf(self->fptr,"%d", &self->real)) {
		status=0;
		wlog("Error reading real value");
		goto _read_header_bail;
	    }
	    if (self->verbose)
		wlog("\treal: %d\n",self->real);
	    if ((self->real != 8) && (self->real != 10)) {
		status=0;
		wlog("Illegal real value (must be 8 or 10)");
		goto _read_header_bail;
	    }	    
        } else if (0 == strcmp(self->buff,"pixelization")) {
            // read the pixelization description, e.g. 9s
            if (1 != fscanf(self->fptr,"%s", self->buff)) {
                status=0;
                wlog("Error reading pixelization scheme");
                goto _read_header_bail;
            }
            if (self->verbose) 
                wlog("\tpixelization scheme: '%s'\n", self->buff);


            if (!pixel_parse_scheme(self->buff, &self->pixelres, &self->pixeltype)) {
                goto _read_header_bail;
            }
            if (self->verbose) {
                wlog("\t\tscheme: '%c'\n", self->pixeltype);
                wlog("\t\tres:     %ld\n", self->pixelres);
            }
        } else {
            status=0;
            wlog("Got unexpected header keyword: '%s'", self->buff);
            goto _read_header_bail;
        }
        if (1 != fscanf(self->fptr,"%s", self->buff) ) {
            status=0;
            wlog("Error reading header keyword");
            goto _read_header_bail;
        }
    }


_read_header_bail:
    return status;
}

int mangle_read_weights(struct MangleMask* self, const char* weightfile)
{
    int status=1;
    FILE *wfptr;
    int64 i;

    long double *weight_new;
    long double test;

    struct Polygon *ply=NULL;



    if ((wfptr = fopen(weightfile,"r")) == NULL) {
        wlog("Failed to open file for reading: %s\n",weightfile);
        status=0;
        goto _mangle_readweight_bail;
    }

    // allocate memory
    if ((weight_new = (long double *)calloc(self->npoly,sizeof(long double))) == NULL) {
        wlog("Failed to allocate memory for reading %s\n",weightfile);
        fclose(wfptr);
        status=0;
        goto _mangle_readweight_bail;
    }

    // read in the lines

    for (i=0;i<self->npoly;i++) {
        if (1 != fscanf(wfptr,"%Lf",&(weight_new[i]))) {
            wlog("Number of weights in weightfile %s less than number of polygons (%ld)\n",weightfile,self->npoly);
            fclose(wfptr);
            status=0;
            free(weight_new);
            goto _mangle_readweight_bail;
        }
    }

    // are there any extra lines?  produce error.
    if (fscanf(wfptr,"%Lf",&test) == 1) {
        wlog("Number of weights in weightfile %s greater than number of polygons (%ld)\n",weightfile,self->npoly);
        fclose(wfptr);
        status=0;
        free(weight_new);
        goto _mangle_readweight_bail;
    }

    // and copy...
    for (i=0;i<self->npoly;i++) {
        ply = &self->poly_vec->data[i];
        ply->weight = weight_new[i];
    }

    // free memory
    free(weight_new);

    // close file
    fclose(wfptr);

    // and because it all worked we can set the filename
    snprintf(self->weightfile,_MANGLE_MAX_FILELEN,"%s",weightfile);

_mangle_readweight_bail:
    return status;
}

int mangle_set_weights(struct MangleMask* self, long double *weights) {
    int status=1;
    int64 i;
    struct Polygon *ply=NULL;

    for (i=0;i<self->npoly;i++) {
        ply = &self->poly_vec->data[i];
        ply->weight = weights[i];
    }

    memset(self->weightfile, 0, sizeof(self->weightfile));

    return status;
}

void mangle_calc_area_and_maxpix(struct MangleMask* self)
{
    struct Polygon* ply=NULL;

    if (self) {
        self->total_area = 0.0;
        if (self->poly_vec) {
            size_t i=0;
            for (i=0; i<self->poly_vec->size; i++) {
                ply = &self->poly_vec->data[i];
                self->total_area += ply->area;

                if (ply->pixel_id > self->maxpix) {
                    self->maxpix = ply->pixel_id;
                }

            }
        }
    }
}

int set_pixel_map(struct MangleMask *self)
{
    int status=1;
    struct Polygon* ply=NULL;
    int64 ipoly=0;

    if (self->pixelres >= 0) {
        if (self->verbose) {
            fprintf(stderr,"Allocating %ld in PixelListVec\n", 
                    self->maxpix+1);
        }
        self->pixel_list_vec = PixelListVec_new(self->maxpix+1);
        if (self->pixel_list_vec == NULL) {
            status = 0;
            goto _set_pixel_map_errout;
        } else {

            if (self->verbose)
                fprintf(stderr,"Filling pixel map\n");

            for (ipoly=0; ipoly<self->poly_vec->size; ipoly++) {
                ply=&self->poly_vec->data[ipoly];
                i64stack_push(self->pixel_list_vec->data[ply->pixel_id], ipoly);
                if (self->verbose > 2) {
                    fprintf(stderr,
                            "Adding poly %ld to pixel map at %ld (%ld)\n",
                            ipoly,ply->pixel_id,
                            self->pixel_list_vec->data[ply->pixel_id]->size);
                }
            }
        }
    }
_set_pixel_map_errout:

    return status;

}

int mangle_polyid_and_weight(struct MangleMask *self, 
                             struct Point *pt, 
                             int64 *poly_id,
                             long double *weight)
{
    if (self->pixeltype == 'u') {
        return mangle_polyid_and_weight_nopix(self,pt,poly_id,weight);
    } else {
        return mangle_polyid_and_weight_pix(self,pt,poly_id,weight);
    }
}

int mangle_polyid_and_weight_nopix(struct MangleMask *self, 
                                   struct Point *pt, 
                                   int64 *poly_id,
                                   long double *weight)
{
    size_t i=0;
    struct Polygon* ply=NULL;

    *poly_id=-1;
    *weight=0.0;

    for (i=0; i<self->poly_vec->size; i++) {
        ply = &self->poly_vec->data[i];
        if (is_in_poly(ply, pt)) {
            *poly_id=ply->poly_id;
            *weight=ply->weight;
            break;
        }
    }
    return 1;
}
int mangle_polyid_and_weight_pix(struct MangleMask *self, 
                                 struct Point *pt, 
                                 int64 *poly_id,
                                 long double *weight)
{
    int status=1;
    size_t i=0;
    int64 pix=0, ipoly=0;
    struct i64stack* pstack=NULL;
    struct Polygon* ply=NULL;

    *poly_id=-1;
    *weight=0.0;

    if (self->pixeltype == 's') {
        pix = get_pixel_simple(self->pixelres, pt);
        if (pix < self->pixel_list_vec->size) {
            // this is a stack holding indices into the polygon vector
            pstack = self->pixel_list_vec->data[pix];

            for (i=0; i<pstack->size; i++) {
                ipoly = pstack->data[i];
                ply = &self->poly_vec->data[ipoly];

                if (is_in_poly(ply, pt)) {
                    *poly_id=ply->poly_id;
                    *weight=ply->weight;
                    break;
                }
            }
        }
    } else {
        status=0;
        wlog("Unsupported pixelization scheme: '%c'",self->pixeltype);
    }
    return status;
}


