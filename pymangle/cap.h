#ifndef _MANGLE_CAP_H
#define _MANGLE_CAP_H

#include <stdio.h>
#include "point.h"

struct Cap {
    double x;
    double y;
    double z;
    double cm;
};

struct CapVec {
    size_t size;
    struct Cap* data;
};


struct CapVec* CapVec_new(size_t n);
struct CapVec* CapVec_free(struct CapVec* self);

int read_cap(FILE* fptr, struct Cap* self);
void print_cap(FILE* fptr, struct Cap* self);

int is_in_cap(struct Cap* cap, struct Point* pt);
#endif
