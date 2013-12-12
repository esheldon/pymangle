#ifndef _MANGLE_POINT_H
#define _MANGLE_POINT_H

struct Point {
    double theta;
    double phi;
    double x;
    double y;
    double z;
};

void point_set_from_radec(struct Point* pt, double ra, double dec);
void point_set_from_thetaphi(struct Point* pt, double theta, double phi);
void radec_from_point(struct Point* pt, double *ra, double *dec);

#endif
