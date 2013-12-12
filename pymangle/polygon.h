#ifndef _MANGLE_POLYGON_H
#define _MANGLE_POLYGON_H

#include <stdio.h>
#include "defs.h"
#include "point.h"
#include "cap.h"

struct Polygon {

    int64 poly_id;
    int64 pixel_id; // optional
    double weight;
    double area; // in str

    struct CapVec* cap_vec;

};

struct PolygonVec {
    size_t size;
    struct Polygon* data;
};

struct PolygonVec* PolygonVec_new(size_t n);
struct PolygonVec* PolygonVec_free(struct PolygonVec* self);
struct PolygonVec *read_polygons(FILE* fptr, size_t npoly);
int read_polygon(FILE* fptr, struct Polygon* ply);
int read_polygon_header(FILE* fptr, struct Polygon* ply, size_t* ncaps);

int is_in_poly(struct Polygon* ply, struct Point* pt);

int scan_expected_value(FILE* fptr, char* buff, const char* expected_value);


void print_polygon(FILE* fptr, struct Polygon* self);
void print_polygons(FILE* fptr, struct PolygonVec *self);

#endif
