#ifndef _MANGLE_CAP_H
#define _MANGLE_CAP_H

#include <stdio.h>
#include "point.h"

struct Cap {
    long double x;
    long double y;
    long double z;
    long double cm;
};

#define CAPVEC_INITCAP 1
#define CAPVEC_PUSH_REALLOC_MULTVAL 2

// ->size and ->data are considered exposed
struct CapVec {
    // the visible size of the vector, can be zero
    size_t size;

    // the capacity of the vector.  For valid vectors this
    // will always be >= 1
    size_t capacity;

    // the underlying data array
    struct Cap* data;
};

// specialized cap for generating random points
// this saves some information needed for each random
struct CapForRand {
    long double theta; // theta in radians (=dec-pi/2)
    long double phi;   // phi in radians (=ra)
    long double cos_theta;
    long double sin_theta;
    long double cos_phi;
    long double sin_phi;

    long double angle; // angular "radius" of cap in radians
                       // can calculate from a Cap with acosl(1-cm)
};

// new capvec with the default capacity (CAPVEC_INITCAP) but size 0
struct CapVec* capvec_new(void);

// new capvec with specified number of elements.   All data
// are zerod
struct CapVec* capvec_zeros(size_t n);

// make sure the backing data array has at least the requested
// number of elements.  The ->size member is unchanged
int capvec_reserve(struct CapVec* self, size_t new_capacity);

// resize the vector; if the new size is larger than the old
// *capacity*, then a reallocation is performed, otherwise
// the underlying data array is not changed, only the size
// member is changed
int capvec_resize(struct CapVec* self, size_t new_size);

// set the size to zero and the reallocate to the default capacity
// (CAPVEC_INITCAP) this means the data should be considered lost
int capvec_clear(struct CapVec* self);

// push a new Cap onto the end of the vector, and increment the size
// member
//
// the data of the cap is copied
//
// the capacity can be changed if the new size exceeds the capacity
// resulting in a reallocation
int capvec_push(struct CapVec* self, const struct Cap* cap);

// return a copy of the last Cap in the vector and decrement the size
// member
//
// If the size is already 0, the return value is just the remaining
// element at position 0, which might be garbage
struct Cap capvec_pop(struct CapVec* self);

// completely free all memory and return NULL
struct CapVec* capvec_free(struct CapVec* self);

// get a full copy of the vector in new CapVec
struct CapVec* capvec_copy(const struct CapVec* self);

/*
   Find the smallest cap in the cap vector

   Adapted from cmminf A J S Hamilton 2001
*/

void capvec_min_cm(const struct CapVec* self,
                   size_t *index,
                   long double* cm_min);

void cap_set(struct Cap* self,
             long double x,
             long double y,
             long double z,
             long double cm);

int read_cap(FILE* fptr, struct Cap* self);
void print_cap(FILE* fptr, struct Cap* self);
void snprint_cap(const struct Cap* self, char *buff, size_t n);

int is_in_cap(struct Cap* cap, struct Point* pt);

/*
   generating random points in a cap.

   A special structure CapForRand is used that holds some
   extra info needed,  the extra info being saved for
   efficiency reasons.
*/

/*
   theta, phi, angle in radians
*/
void CapForRand_from_thetaphi(struct CapForRand *rcap,
                              long double theta,
                              long double phi,
                              long double angle_radians);
/*
   ra, dec, angle in degrees
*/
void CapForRand_from_radec(struct CapForRand *rcap,
                           long double ra,
                           long double dec,
                           long double angle_degrees);


void genrand_cap_thetaphi(const struct CapForRand *rcap,
                          int quadrant,
                          long double *theta,
                          long double *phi);
void genrand_cap_radec(const struct CapForRand *rcap,
                       int quadrant,
                       long double *ra,
                       long double *dec);

#endif
