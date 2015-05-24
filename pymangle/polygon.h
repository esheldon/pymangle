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

    struct CapVec* cap_vec;

};

struct Polygon* polygon_new(void);
struct Polygon* polygon_zeros(size_t n);
struct Polygon* polygon_free(struct Polygon* self);
struct Polygon* polygon_copy(const struct Polygon* self);

int polygon_reserve(struct Polygon* self, size_t new_capacity);
int polygon_resize(struct Polygon* self, size_t new_size);

// All metadata reset, caps cleared
int polygon_clear(struct Polygon* self);

int polygon_push_cap(struct Polygon* self, const struct Cap* cap);
struct Cap polygon_pop_cap(struct Polygon* self);


int read_into_polygon(FILE* fptr, struct Polygon* ply);
int read_polygon_header(FILE* fptr, struct Polygon* ply, size_t* ncaps);

int is_in_poly(struct Polygon* ply, struct Point* pt);

int scan_expected_value(FILE* fptr, char* buff, const char* expected_value);


void print_polygon(FILE* fptr, struct Polygon* self);

struct PolygonVec {
    size_t size;
    struct Polygon* data;
};

struct PolygonVec* PolygonVec_new(size_t n);
struct PolygonVec* PolygonVec_free(struct PolygonVec* self);
struct PolygonVec *read_polygons(FILE* fptr, size_t npoly);
void print_polygons(FILE* fptr, struct PolygonVec *self);

#endif
