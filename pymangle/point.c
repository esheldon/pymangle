#include <stdlib.h>
#include <math.h>

#include "mangle.h"
#include "point.h"

void 
point_set_from_radec(struct Point* pt, double ra, double dec) {

    double stheta=0;

    if (pt != NULL) {
        pt->phi = ra*D2R;
        pt->theta = (90.0-dec)*D2R;

        stheta = sin(pt->theta);
        pt->x = stheta*cos(pt->phi);
        pt->y = stheta*sin(pt->phi);
        pt->z = cos(pt->theta); 
    }
}
void 
point_set_from_thetaphi(struct Point* pt, double theta, double phi) {

    double stheta=0;

    if (pt != NULL) {
        pt->phi = phi;
        pt->theta = theta;

        stheta = sin(pt->theta);
        pt->x = stheta*cos(pt->phi);
        pt->y = stheta*sin(pt->phi);
        pt->z = cos(pt->theta); 
    }
}
void 
radec_from_point(struct Point* pt, double *ra, double *dec) {
    *ra = pt->phi*R2D;
    *dec = 90.0 - pt->theta*R2D;
}

