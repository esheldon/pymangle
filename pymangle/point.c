#include <stdlib.h>
#include <math.h>

#include "mangle.h"
#include "point.h"

void 
point_set_from_radec(struct Point* pt, long double ra, long double dec) {

    long double stheta=0;

    if (pt != NULL) {
        pt->phi = ra*D2R;
        pt->theta = (90.0-dec)*D2R;

        stheta = sinl(pt->theta);
        pt->x = stheta*cosl(pt->phi);
        pt->y = stheta*sinl(pt->phi);
        pt->z = cosl(pt->theta); 
    }
}
void 
point_set_from_thetaphi(struct Point* pt, long double theta, long double phi) {

    long double stheta=0;

    if (pt != NULL) {
        pt->phi = phi;
        pt->theta = theta;

        stheta = sinl(pt->theta);
        pt->x = stheta*cosl(pt->phi);
        pt->y = stheta*sinl(pt->phi);
        pt->z = cosl(pt->theta); 
    }
}
void 
radec_from_point(struct Point* pt, long double *ra, long double *dec) {
    *ra = pt->phi*R2D;
    *dec = 90.0 - pt->theta*R2D;
}

