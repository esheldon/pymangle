#ifndef _MANGLE_POINT_H
#define _MANGLE_POINT_H

struct Point {
    long double theta;
    long double phi;
    long double x;
    long double y;
    long double z;
};

void point_set_from_radec(struct Point* pt, long double ra, long double dec);
void point_set_from_thetaphi(struct Point* pt, long double theta, long double phi);
void radec_from_point(struct Point* pt, long double *ra, long double *dec);

#endif
