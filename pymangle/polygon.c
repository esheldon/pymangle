#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "polygon.h"
#include "cap.h"
#include "point.h"

struct Polygon* polygon_new(void)
{
    struct Polygon* self=NULL;

    self=calloc(1, sizeof(struct Polygon));
    if (self==NULL) {
        return NULL;
    }

    self->poly_id=-9999;
    self->pixel_id=-9999;
    self->weight=-9999;
    self->area=-9999;

    self->caps = capvec_new();
    if (self->caps==NULL) {
        free(self);
        return NULL;
    }

    return self;
}

struct Polygon* polygon_zeros(size_t n)
{
    struct Polygon* self=NULL;

    self=calloc(1, sizeof(struct Polygon));
    if (self==NULL) {
        return NULL;
    }

    self->poly_id=-9999;
    self->pixel_id=-9999;
    self->weight=-9999;
    self->area=-9999;

    self->caps = capvec_zeros(n);
    if (self->caps==NULL) {
        free(self);
        return NULL;
    }

    return self;
}


struct Polygon* polygon_free(struct Polygon* self)
{
    if (self) {
        if (self->caps) {
            self->caps=capvec_free(self->caps);
        }
        free(self);
        self=NULL;
    }
    return self;
}

struct Polygon* polygon_copy(const struct Polygon* self)
{
    struct Polygon* poly=NULL;

    poly=calloc(1, sizeof(struct Polygon));
    if (!poly) {
        return NULL;
    }
    // copy  metadata
    memcpy(poly, self, sizeof(struct Polygon));

    // now copy full cap vector
    poly->caps = capvec_copy(self->caps);
    if (!poly->caps) {
        free(poly);
        poly=NULL;
        return poly;
    }

    return poly;
}

int polygon_reserve(struct Polygon* self, size_t new_capacity)
{
    int status=0;

    status=capvec_reserve( self->caps, new_capacity );

    return status;
}

int polygon_resize(struct Polygon* self, size_t new_size)
{
    int status=0;

    status=capvec_resize( self->caps, new_size );
    // no need to alter area since zeros were added

    return status;
}

int polygon_clear(struct Polygon* self)
{
    int status=0;

    self->poly_id=-9999;
    self->pixel_id=-9999;
    self->weight=-9999;
    self->area=-9999;
    self->area_set=0;

    status=capvec_clear( self->caps );

    return status;
}

int polygon_push_cap(struct Polygon* self, const struct Cap* cap)
{
    int status=0;

    status = capvec_push(self->caps, cap);
    return status;
}

// expanding this code to avoid an extra copy
struct Cap polygon_pop_cap(struct Polygon* self)
{
    size_t index=0;
    
    if (self->caps->size > 0) {
        index = self->caps->size-1;
        self->caps->size--;
    } else {
        fprintf(stderr,
                "CapVecError: attempt to pop from empty vector, returning garbage\n");
    }

    return self->caps->data[index];
}


int polygon_has_zero_area(const struct Polygon* self) {
    size_t i=0;
    int has_zero_area=0;
    struct Cap* cap=NULL;

    for (i=0; i<self->caps->size; i++) {
        cap=&self->caps->data[i];
        if (cap->cm == 0.0L || cap->cm <= -2.0L) {
            has_zero_area=1;
            break;
        }
    }

    return has_zero_area;
}

int is_in_poly(struct Polygon* ply, struct Point* pt)
{
    size_t i=0;
    struct Cap* cap=NULL;

    int inpoly=1;


    for (i=0; i<ply->caps->size; i++) {
        cap = &ply->caps->data[i];
        inpoly = inpoly && is_in_cap(cap, pt);
        if (!inpoly) {
            break;
        }
    }
    return inpoly;
}


/*
long double polygon_calc_area(const struct CapVec* self, long double *tol)
{
    long double area=0;
    // static work structure
    static struct CapVec* dcaps=NULL;
    size_t index=0;
    long double cm_min=0;
    long double darea=0;

    capvec_min_cm(self, &index, &cm_min);


    return area;
}
*/

int read_into_polygon(FILE* fptr, struct Polygon* ply)
{
    int status=1;
    struct Cap* cap=NULL;

    size_t ncaps=0, i=0;

    if (!read_polygon_header(fptr, ply, &ncaps)) {
        status=0;
        goto _read_single_polygon_errout;
    }

    if (ply->caps) {
        capvec_resize(ply->caps, ncaps);
    } else {
        ply->caps = capvec_zeros(ncaps);
        if (ply->caps == NULL) {
            status=0;
            goto _read_single_polygon_errout;
        }
    }

    for (i=0; i<ncaps; i++) {
        cap = &ply->caps->data[i];
        status = read_cap(fptr, cap);
        if (status != 1) {
            goto _read_single_polygon_errout;
        }
    }

_read_single_polygon_errout:
    return status;
}

/* 
 * parse the polygon "header" for the index poly_index
 *
 * this is after reading the initial 'polygon' token
 */

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

    if (1 != fscanf(fptr,"%Lf",&ply->weight)) {
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
        sscanf(valbuff, "%Lf", &ply->area);
        ply->area_set=1;
    }
    if (got_pixel) {
        if (1 != fscanf(fptr,"%Lf",&ply->area)) {
            status=0;
            wlog("Failed to read area for polygon id %ld", 
                 ply->poly_id);
            goto _read_polygon_header_errout;
        }
        ply->area_set=1;
        if (!scan_expected_value(fptr, kwbuff, "str):")) {
            status=0;
            goto _read_polygon_header_errout;
        }
    }

_read_polygon_header_errout:
    return status;
}

void print_polygon(FILE* fptr, struct Polygon* self)
{
    if (!self)
        return;

    size_t ncaps=0;
    struct CapVec* caps=NULL;
    caps = self->caps;
    ncaps = caps ? caps->size : 0;

    fprintf(fptr,
            "polygon %ld ( %ld caps, %.18Lg weight, %ld pixel, %.18Lg str):\n",
            self->poly_id, ncaps, self->weight, 
            self->pixel_id, self->area);
    if (ncaps > 0) {
        size_t i=0;
        for (i=0; i<ncaps; i++) {
            struct Cap *cap = &caps->data[i];
            print_cap(fptr,cap);
        }
    }

}


struct PolyVec* polyvec_new(size_t n) 
{
    struct PolyVec* self=NULL;

    self=calloc(1, sizeof(struct PolyVec));
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

struct PolyVec* polyvec_free(struct PolyVec* self)
{
    struct Polygon* ply=NULL;
    size_t i=0;
    if (self != NULL) {
        if (self->data!= NULL) {

            for (i=0; i<self->size; i++) {
                ply=&self->data[i];
                ply->caps = capvec_free(ply->caps);
            }
            free(self->data);

        }
        free(self);
        self=NULL;
    }
    return self;
}

struct PolyVec *read_polygons(FILE* fptr, size_t npoly)
{
    int status=1;
    char buff[_MANGLE_SMALL_BUFFSIZE];
    struct PolyVec *self=NULL;
    size_t i=0;

    self = polyvec_new(npoly);
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

        status = read_into_polygon(fptr, &self->data[i]);
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



void print_polygons(FILE* fptr, struct PolyVec *self)
{
    if (self) {
        size_t i;
        struct Polygon* ply=NULL;

        for (i=0; i<self->size; i++) {
            ply = &self->data[i];
            print_polygon(fptr,ply);
        }
    }
}
