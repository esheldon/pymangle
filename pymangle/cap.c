#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "cap.h"
#include "point.h"
#include "defs.h"

struct CapVec* 
CapVec_new(size_t n) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        return NULL;
    }
    self->data = calloc(n, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        return NULL;
    }
    self->size = n;
    return self;
}

struct CapVec* CapVec_free(struct CapVec* self)
{
    if (self != NULL) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}

void cap_set(struct Cap* self,
             long double x,
             long double y,
             long double z,
             long double cm)
{
    self->x=x;
    self->y=y;
    self->z=z;
    self->cm=cm;
}


int read_cap(FILE* fptr, struct Cap* self) 
{
    int status=1, nres=0;

    nres = fscanf(fptr,
            "%Lf %Lf %Lf %Lf",&self->x,&self->y,&self->z,&self->cm);

    if (nres != 4) {
        status=0;
        wlog("Failed to read cap\n");
    }

    return status;
}

void print_cap(FILE* fptr, struct Cap* self)
{
    if (!self)
        return;

    fprintf(fptr, "  %.18Lg %.18Lg %.18Lg %.18Lg\n", 
            self->x, self->y, self->z, self->cm);
}

void snprint_cap(const struct Cap* self, char *buff, size_t n)
{
    if (!self)
        return;

    snprintf(buff,
             n,
             "%.18Lg %.18Lg %.18Lg %.18Lg",
             self->x,
             self->y,
             self->z,
             self->cm);
}



int is_in_cap(struct Cap* cap, struct Point* pt)
{
    int incap=0;
    long double cdot;

    cdot = 1.0 - cap->x*pt->x - cap->y*pt->y - cap->z*pt->z;
    if (cap->cm < 0.0) {
        incap = cdot > (-cap->cm);
    } else {
        incap = cdot < cap->cm;
    }

    return incap;
}

void CapForRand_from_thetaphi(struct CapForRand *rcap,
                              long double theta,
                              long double phi,
                              long double angle_radians)
{
    rcap->theta=theta;
    rcap->phi=phi;
    rcap->cos_theta = cosl(theta);
    rcap->sin_theta = sinl(theta);
    rcap->cos_phi = cosl(phi);
    rcap->sin_phi = sinl(phi);
    rcap->angle = angle_radians;
}
void CapForRand_from_radec(struct CapForRand *rcap,
                           long double ra,
                           long double dec,
                           long double angle_degrees)
{
    long double theta, phi;
    thetaphi_from_radec(ra, dec, &theta, &phi);

    CapForRand_from_thetaphi(rcap, theta, phi, angle_degrees*D2R);
}

void genrand_cap_thetaphi(const struct CapForRand *rcap,
                          int quadrant,
                          long double *theta,
                          long double *phi)
{
    long double rand_r, rand_posangle, 
        sinr, cosr, cospsi, sintheta, costheta,
        cosDphi, Dphi;

    // uniform in opening angle squared
    rand_r = (long double) ( sqrt(drand48())*rcap->angle );

    rand_posangle = (long double) ( drand48()*2*M_PI );
    switch (quadrant) {
        case 1: 
            rand_posangle *= 0.25; // scale back to range pi/2
            break;
        case 2: 
            rand_posangle *= 0.25; // scale back to range pi/2
            rand_posangle += 0.5*M_PI; // translate to [pi/2,pi]
            break;
        case 3: 
            rand_posangle *= 0.25; // scale back to range pi/2
            rand_posangle += M_PI; // translate to [pi, 3*pi/2]
            break;
        case 4: 
            rand_posangle *= 0.25; // scale back to range pi/2
            rand_posangle += 1.5*M_PI; // translate to [3*pi/2, 2*pi]
            break;
        default:
            // keep full cap
            break;
    }

    sinr = sinl(rand_r);
    cosr = cosl(rand_r);

    cospsi = cosl(rand_posangle);
    costheta = rcap->cos_theta*cosr + rcap->sin_theta*sinr*cospsi;
    if (costheta < -1.) {
        costheta = -1.;
    } else if (costheta > 1.) {
        costheta =  1.;
    }

    (*theta) = acosl(costheta);
    sintheta = sinl(*theta);

    cosDphi = (cosr - rcap->cos_theta*costheta)/(rcap->sin_theta*sintheta);

    if (cosDphi < -1.) {
        cosDphi = -1.;
    } else if (cosDphi > 1.) {
        cosDphi =  1.;
    }
    Dphi = acosl(cosDphi);

    if (rand_posangle > M_PI) {
        (*phi) = rcap->phi + Dphi;
    } else {
        (*phi) = rcap->phi - Dphi;
    }
}

void genrand_cap_radec(const struct CapForRand *rcap,
                       int quadrant,
                       long double *ra,
                       long double *dec)
{
    long double theta, phi;
    int quadrant_thetaphi;

    switch (quadrant) {
        case 1: 
            quadrant_thetaphi = 4;
            break;
        case 2: 
            quadrant_thetaphi = 1;
            break;
        case 3: 
            quadrant_thetaphi = 2;
            break;
        case 4: 
            quadrant_thetaphi = 3;
            break;
        default:
            quadrant_thetaphi=quadrant;
            break;
    }


    genrand_cap_thetaphi(rcap, quadrant_thetaphi, &theta, &phi);
    radec_from_thetaphi(theta, phi, ra, dec);
}
