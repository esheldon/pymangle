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

struct CapVec {
    size_t size;
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

struct CapVec* CapVec_new(size_t n);
struct CapVec* CapVec_free(struct CapVec* self);

int read_cap(FILE* fptr, struct Cap* self);
void print_cap(FILE* fptr, struct Cap* self);

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
