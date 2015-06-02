#ifndef _MANGLE_POLYGON_H
#define _MANGLE_POLYGON_H

#include <stdio.h>
#include "defs.h"
#include "point.h"
#include "cap.h"

struct Polygon {

    // these considered exposed
    int64 poly_id;
    int64 pixel_id; // optional
    long double weight;

    int area_set;
    long double area; // in str

    struct CapVec* caps;

};

// new polygon with capvec that has the default capacity (CAPVEC_INITCAP) but size 0
// the metadata is -9999 with ->area_set==0
struct Polygon* polygon_new(void);

// similar to new() but n caps zerod according to capvec_zeros(n)
struct Polygon* polygon_zeros(size_t n);

// completely free all data
struct Polygon* polygon_free(struct Polygon* self);

// full copy of all metadata and caps
struct Polygon* polygon_copy(const struct Polygon* self);

// reserve the specified number of caps
int polygon_reserve(struct Polygon* self, size_t new_capacity);

// resize the underlying cap vector
int polygon_resize(struct Polygon* self, size_t new_size);

// All metadata reset, caps cleared
int polygon_clear(struct Polygon* self);

// push a new cap onto the polygon (underlying caps vector)
int polygon_push_cap(struct Polygon* self, const struct Cap* cap);

// return a copy of the last Cap in the caps vector and decrement the size
// member
//
// If the size is already 0, the return value is just the remaining element at
// position 0, which might be garbage

struct Cap polygon_pop_cap(struct Polygon* self);

// loop through caps and see if any have zero area.
// this does not use the ->area member
// adapted from gzeroar, A J S Hamilton
int polygon_has_zero_area(const struct Polygon* self);

int read_into_polygon(FILE* fptr, struct Polygon* ply);
int read_polygon_header(FILE* fptr, struct Polygon* ply, size_t* ncaps);

int is_in_poly(struct Polygon* ply, struct Point* pt);

int scan_expected_value(FILE* fptr, char* buff, const char* expected_value);


void print_polygon(FILE* fptr, struct Polygon* self);

struct PolyVec {
    size_t size;
    struct Polygon* data;
};

struct PolyVec* polyvec_new(size_t n);
struct PolyVec* polyvec_free(struct PolyVec* self);
struct PolyVec *read_polygons(FILE* fptr, size_t npoly);
void print_polygons(FILE* fptr, struct PolyVec *self);

#endif
