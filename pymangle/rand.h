#ifndef _MANGLE_RAND_H
#define _MANGLE_RAND_H

#include "point.h"
void seed_random(void);

void genrand_allsky(struct Point *pt);
void genrand_range(long double cthmin, long double cthmax, 
                   long double phimin, long double phimax,
                   struct Point *pt);

void genrand_theta_phi_allsky(long double* theta, long double* phi);

void genrand_theta_phi(long double cthmin, long double cthmax, 
                       long double phimin, long double phimax,
                       long double* theta, long double* phi);

/*
 * convert an ra/dec range to cos(theta) and phi range
 * for quicker use by the random range generators
 */

int
radec_range_to_costhetaphi(long double ramin, long double ramax, 
                           long double decmin, long double decmax,
                           long double* cthmin, long double* cthmax, 
                           long double* phimin, long double* phimax);

#endif
