#include <stdlib.h>
#include <stdio.h>
#include "cap.h"
#include "point.h"
#include "defs.h"

struct CapVec* 
CapVec_new(size_t n) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        return NULL;
    }
    self->data = calloc(n, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        return NULL;
    }
    self->size = n;
    return self;
}

struct CapVec* CapVec_free(struct CapVec* self)
{
    if (self != NULL) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}

int read_cap(FILE* fptr, struct Cap* self) 
{
    int status=1, nres=0;

    nres = fscanf(fptr,
            "%lf %lf %lf %lf",&self->x,&self->y,&self->z,&self->cm);

    if (nres != 4) {
        status=0;
        wlog("Failed to read cap\n");
    }

    return status;
}
void print_cap(FILE* fptr, struct Cap* self)
{
    if (!self)
        return;

    fprintf(fptr, "  %.16g %.16g %.16g %.16g\n", 
            self->x, self->y, self->z, self->cm);
}

int is_in_cap(struct Cap* cap, struct Point* pt)
{
    int incap=0;
    double cdot;

    cdot = 1.0 - cap->x*pt->x - cap->y*pt->y - cap->z*pt->z;
    if (cap->cm < 0.0) {
        incap = cdot > (-cap->cm);
    } else {
        incap = cdot < cap->cm;
    }

    return incap;
}

