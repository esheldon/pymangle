#ifndef _MANGLE_RAND_H
#define _MANGLE_RAND_H

#include "point.h"
void seed_random(void);

void genrand_allsky(struct Point *pt);
void genrand_range(double cthmin, double cthmax, 
                   double phimin, double phimax,
                   struct Point *pt);

void genrand_theta_phi_allsky(double* theta, double* phi);

void genrand_theta_phi(double cthmin, double cthmax, 
                       double phimin, double phimax,
                       double* theta, double* phi);

/*
 * convert an ra/dec range to cos(theta) and phi range
 * for quicker use by the random range generators
 */

int
radec_range_to_costhetaphi(double ramin, double ramax, 
                           double decmin, double decmax,
                           double* cthmin, double* cthmax, 
                           double* phimin, double* phimax);

#endif
