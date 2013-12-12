#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "polygon.h"
#include "cap.h"
#include "point.h"

struct PolygonVec* PolygonVec_new(size_t n) 
{
    struct PolygonVec* self=NULL;

    self=calloc(1, sizeof(struct PolygonVec));
    if (self == NULL) {
        return NULL;
    }
    // pointers will be NULL (0)
    self->data = calloc(n, sizeof(struct Polygon));
    if (self->data == NULL) {
        free(self);
        return NULL;
    }

    self->size = n;
    return self;
}

struct PolygonVec* PolygonVec_free(struct PolygonVec* self)
{
    struct Polygon* ply=NULL;
    size_t i=0;
    if (self != NULL) {
        if (self->data!= NULL) {

            ply=self->data;
            for (i=0; i<self->size; i++) {
                ply->cap_vec = CapVec_free(ply->cap_vec);
                ply++;
            }
            free(self->data);

        }
        free(self);
        self=NULL;
    }
    return self;
}

int is_in_poly(struct Polygon* ply, struct Point* pt)
{
    size_t i=0;
    struct Cap* cap=NULL;

    int inpoly=1;


    cap = &ply->cap_vec->data[0];
    for (i=0; i<ply->cap_vec->size; i++) {
        inpoly = inpoly && is_in_cap(cap, pt);
        if (!inpoly) {
            break;
        }
        cap++;
    }
    return inpoly;
}

struct PolygonVec *read_polygons(FILE* fptr, size_t npoly)
{
    int status=1;
    char buff[_MANGLE_SMALL_BUFFSIZE];
    struct PolygonVec *self=NULL;
    size_t i=0;

    self = PolygonVec_new(npoly);
    if (!self) {
        status=0;
        wlog("could not allocate %lu polygons\n", npoly);
        goto _read_polygons_bail;
    }

    // in order to get here, we had to read the token already
    strcpy(buff, "polygon");

    for (i=0; i<npoly; i++) {
        // buff comes in with 'polygon'
        if (0 != strcmp(buff,"polygon")) {
            status=0;
            wlog("Expected first token in polygon %lu to read "
                 "'polygon', got '%s'\n", i, buff);
            goto _read_polygons_bail;
        }

        status = read_polygon(fptr, &self->data[i]);
        if (!status) {
            wlog("failed to read polygon %lu\n", i);
            break;
        }

        if (i != (npoly-1)) {
            if (1 != fscanf(fptr,"%s",buff)) {
                status=0;
                wlog("Error reading token for polygon %lu\n", i);
                goto _read_polygons_bail;
            }
        }
    }

_read_polygons_bail:

    if (!status) {
        free(self);
        self=NULL;
    }
    return self;
}


int read_polygon(FILE* fptr, struct Polygon* ply)
{
    int status=1;
    struct Cap* cap=NULL;

    size_t ncaps=0, i=0;

    if (!read_polygon_header(fptr, ply, &ncaps)) {
        status=0;
        goto _read_single_polygon_errout;
    }

    ply->cap_vec = CapVec_new(ncaps);
    if (ply->cap_vec == NULL) {
        status=0;
        goto _read_single_polygon_errout;
    }

    cap = &ply->cap_vec->data[0];
    for (i=0; i<ncaps; i++) {
        status = read_cap(fptr, cap);
        if (status != 1) {
            goto _read_single_polygon_errout;
        }
        cap++;
    }
_read_single_polygon_errout:
    return status;
}

/* 
 * parse the polygon "header" for the index poly_index
 *
 * this is after reading the initial 'polygon' token
 */
int read_polygon_header(FILE* fptr, struct Polygon* ply, size_t* ncaps)
{
    int status=1;
    int got_pixel=0;
    char kwbuff[20];
    char valbuff[25];

    if (1 != fscanf(fptr, "%ld", &ply->poly_id)) {
        status=0;
        wlog("Failed to read polygon id\n");
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, kwbuff, "(")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%ld",ncaps)) {
        status=0;
        wlog("Failed to read ncaps for polygon id %ld", 
             ply->poly_id);
        goto _read_polygon_header_errout;
    }


    if (!scan_expected_value(fptr, kwbuff, "caps,")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    if (1 != fscanf(fptr,"%lf",&ply->weight)) {
        status=0;
        wlog("Failed to read weight for polygon id %ld", 
             ply->poly_id);
        goto _read_polygon_header_errout;
    }

    if (!scan_expected_value(fptr, kwbuff, "weight,")) {
        status=0;
        goto _read_polygon_header_errout;
    }

    // pull in the value and keyword
    if (2 != fscanf(fptr,"%s %s",valbuff, kwbuff)) {
        status=0;
        wlog("Failed to read value and keyword (pixel,str) "
             "for polygon id %ld\n", 
             ply->poly_id);
        goto _read_polygon_header_errout;
    }

    if (0 == strcmp(kwbuff,"pixel,")) {
        // we read a pixel value into valbuff
        got_pixel=1;
        sscanf(valbuff, "%ld", &ply->pixel_id);
    } else {
        // we probably read the area
        if (0 != strcmp(kwbuff,"str):")) {
            status=0;
            wlog("Expected str): keyword at polygon id %ld, got %s", 
                 ply->poly_id, kwbuff);
            goto _read_polygon_header_errout;
        }
        sscanf(valbuff, "%lf", &ply->area);
    }
    if (got_pixel) {
        if (1 != fscanf(fptr,"%lf",&ply->area)) {
            status=0;
            wlog("Failed to read area for polygon id %ld", 
                 ply->poly_id);
            goto _read_polygon_header_errout;
        }
        if (!scan_expected_value(fptr, kwbuff, "str):")) {
            status=0;
            goto _read_polygon_header_errout;
        }
    }

    /*
    if (ply->pixel_id > self->maxpix) {
        self->maxpix = ply->pixel_id;
    }
    self->total_area += ply->area;

    if (self->verbose > 1) {
        fprintf(stderr,
          "polygon %ld: poly_id %ld ncaps: %ld weight: %g pixel: %ld area: %g\n", 
          self->current_poly_index, ply->poly_id, *ncaps, ply->weight, 
          ply->pixel_id, ply->area);
    }
    */

_read_polygon_header_errout:
    return status;
}

int scan_expected_value(FILE* fptr, char* buff, const char* expected_value)
{
    int status=1, res=0;

    res = fscanf(fptr, "%s", buff);
    if (1 != res || 0 != strcmp(buff,expected_value)) {
        status=0;
        wlog("Failed to read expected string in polygon: '%s' "
             "got '%s'", expected_value, buff);
    }
    return status;
}


void print_polygon(FILE* fptr, struct Polygon* self)
{
    if (!self)
        return;

    size_t ncaps=0;
    struct CapVec* cap_vec=NULL;
    cap_vec = self->cap_vec;
    ncaps = cap_vec ? cap_vec->size : 0;

    fprintf(fptr,
            "polygon %ld ( %ld caps, %.16g weight, %ld pixel, %.16g str):\n",
            self->poly_id, ncaps, self->weight, 
            self->pixel_id, self->area);
    if (ncaps > 0) {
        size_t i=0;
        struct Cap *cap = &cap_vec->data[0];
        for (i=0; i<ncaps; i++) {
            print_cap(fptr,cap);
            cap++;
        }
    }

}
void print_polygons(FILE* fptr, struct PolygonVec *self)
{
    if (self) {
        size_t i;
        struct Polygon* ply=NULL;

        ply = &self->data[0];
        for (i=0; i<self->size; i++) {
            print_polygon(fptr,ply);
            ply++;
        }
    }
}
