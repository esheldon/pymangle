#ifndef _MANGLE_POINT_H
#define _MANGLE_POINT_H

struct Point {
    long double theta; // theta in radians (=dec-pi/2)
    long double phi;   // phi in radians (=ra)
    long double x;
    long double y;
    long double z;
};

void point_set_from_radec(struct Point* pt, long double ra, long double dec);
void point_set_from_thetaphi(struct Point* pt, long double theta, long double phi);
void radec_from_point(struct Point* pt, long double *ra, long double *dec);

/*
    ra = phi*R2D;
    dec = 90.0 - theta*R2D;
*/
void radec_from_thetaphi(long double theta,
                         long double phi,
                         long double *ra,
                         long double *dec);
/*
    theta = (90.0-dec)*D2R;
    phi   = ra*D2R;
*/
void thetaphi_from_radec(long double ra,
                         long double dec,
                         long double *theta,
                         long double *phi);

#endif
