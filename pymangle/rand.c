#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "rand.h"
#include "point.h"
#include "defs.h"

void
seed_random(void) {
    struct timeval tm;
    gettimeofday(&tm, NULL); 
    srand48((long) (tm.tv_sec * 1000000 + tm.tv_usec));
}


void genrand_allsky(struct Point *pt)
{
    double theta, phi;
    genrand_theta_phi_allsky(&theta, &phi);
    point_set_from_thetaphi(pt, theta, phi);
}
void genrand_range(double cthmin, double cthmax, 
                   double phimin, double phimax,
                   struct Point *pt)
{
    double theta, phi;
    genrand_theta_phi(cthmin, cthmax, 
                      phimin, phimax,
                      &theta, &phi);

    point_set_from_thetaphi(pt, theta, phi);
}
int 
radec_range_to_costhetaphi(double ramin, double ramax, 
                           double decmin, double decmax,
                           double* cthmin, double* cthmax, 
                           double* phimin, double* phimax)
{

    if (ramin < 0.0 || ramax > 360.0) {
        wlog("ra range must be in [0,360] got [%.16g,%.16g]",
                ramin,ramax);
        return 0;
    }
    if (decmin < -90.0 || decmax > 90.0) {
        wlog("dec range must be in [-90,90] got [%.16g,%.16g]",
             decmin,decmax);
        return 0;
    }

    *cthmin = cos((90.0-decmin)*D2R);
    *cthmax = cos((90.0-decmax)*D2R);
    *phimin = ramin*D2R;
    *phimax = ramax*D2R;
    return 1;
}



/*
 * constant in cos(theta)
 */
void
genrand_theta_phi_allsky(double* theta, double* phi)
{
    *phi = drand48()*2*M_PI;
    // this is actually cos(theta) for now
    *theta = 2*drand48()-1;
    
    if (*theta > 1) *theta=1;
    if (*theta < -1) *theta=-1;

    *theta = acos(*theta);
}

/*
 * Generate random points in a range.  Inputs are
 * min(cos(theta)), max(cos(theta)), min(phi), max(phi)
 *
 * constant in cos(theta)
 */
void
genrand_theta_phi(double cthmin, double cthmax, double phimin, double phimax,
                  double* theta, double* phi)
{

    // at first, theta is cos(theta)
    *phi = phimin + (phimax - phimin)*drand48();

    // this is actually cos(theta) for now
    *theta = cthmin + (cthmax-cthmin)*drand48();
    
    if (*theta > 1) *theta=1;
    if (*theta < -1) *theta=-1;

    *theta = acos(*theta);
}

