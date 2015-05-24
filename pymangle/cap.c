#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "cap.h"
#include "point.h"
#include "defs.h"

/*
   Cap code
*/

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
    long double cdotm=0;

    // cm = 1-cos(theta) where theta is the cap opening angle
    cdotm = 1.0 - cap->x*pt->x - cap->y*pt->y - cap->z*pt->z;
    if (cap->cm < 0.0) {
        incap = cdotm > (-cap->cm);
    } else {
        incap = cdotm < cap->cm;
    }

    return incap;
}

/*

   CapVec code

*/

struct CapVec* capvec_new(void) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        return NULL;
    }

    self->capacity = CAPVEC_INITCAP;
    self->size = 0;

    self->data = calloc(self->capacity, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        return NULL;
    }
    return self;
}

struct CapVec* capvec_zeros(size_t n) 
{
    struct CapVec* self=NULL;

    self=calloc(1, sizeof(struct CapVec));
    if (self == NULL) {
        return NULL;
    }

    self->capacity = n;
    self->size = n;

    self->data = calloc(self->capacity, sizeof(struct Cap));
    if (self->data == NULL) {
        free(self);
        return NULL;
    }

    return self;
}

struct CapVec* capvec_free(struct CapVec* self)
{
    if (self != NULL) {
        if (self->data) {
            free(self->data);
            self->data=NULL;
        }
        free(self);
        self=NULL;
    }
    return self;
}


static int capvec_realloc(struct CapVec* self, size_t new_capacity)
{
    size_t old_capacity=self->capacity;

    if (new_capacity < old_capacity) {
        self->size=new_capacity;
    }

    if (new_capacity < 1) new_capacity=1;

    size_t sizeof_type = sizeof(struct Cap);

    if (new_capacity != old_capacity) {
        self->data = realloc(self->data, new_capacity*sizeof_type);
        if (!self->data) {
            fprintf(stderr, "failed to reallocate\n");
            return 0;
        }
        if (new_capacity > old_capacity) {
            // zero out additional elements if new capacity is larger
            size_t num_new_bytes = (new_capacity-old_capacity)*sizeof_type;
            memset(self->data + old_capacity, 0, num_new_bytes);
        }

        self->capacity = new_capacity;
    }

    return 1;
}

int capvec_reserve(struct CapVec* self, size_t new_capacity)
{
    int status=1;
    if (new_capacity > self->capacity) {
        status=capvec_realloc(self, new_capacity);
    }
    return status;
}

int capvec_resize(struct CapVec* self, size_t new_size)
{
    int status=1;

    // if new size is smaller, we will just leave junk in the 
    // unused slots, which is OK since they are not "visible".
    //
    // If size is larger, only new capacity is zerod by reserve
    // so we want to zero any elements up to current capacity

    if (new_size > self->size) {
        if (self->size < self->capacity) {
            size_t n_to_zero=self->capacity-self->size;
            memset(&self->data[self->size],
                   0,
                   n_to_zero*sizeof(struct Cap));
        }
    }

    // get at least new_size capacity
    status=capvec_reserve(self, new_size);
    if (status) {
        self->size=new_size;
    }

    return status;
}

int capvec_clear(struct CapVec* self)
{
    int status=0;
    status=capvec_realloc(self, CAPVEC_INITCAP);

    if (status) {
        self->size=0;
    }

    return status;
}

int capvec_push(struct CapVec* self, const struct Cap* cap)
{
    int status=1;
    if (self->size == self->capacity) {
        size_t new_capacity=0;
        if (self->capacity==0) {
            new_capacity=CAPVEC_INITCAP;
        } else {
            new_capacity = self->capacity*CAPVEC_PUSH_REALLOC_MULTVAL;
        }
        status=capvec_realloc(self, new_capacity);
    }

    if (status) {
        self->size++;
        self->data[self->size-1] = *cap;
    }

    return status;
}

struct Cap capvec_pop(struct CapVec* self)
{
    size_t index=0;
    
    if (self->size > 0) {
        index = self->size-1;
        self->size--;
    } else {
        fprintf(stderr,
                "CapVecError: attempt to pop from empty vector, returning garbage\n");
    }

    return self->data[index];
}

struct CapVec* capvec_copy(const struct CapVec* self)
{
    struct CapVec * cap_vec=NULL;
    if (!self) {
        return NULL;
    }

    cap_vec = capvec_zeros(self->size);
    memcpy(cap_vec->data,
           self->data,
           self->size*sizeof(struct Cap));

    return cap_vec;
}

void capvec_min_cm(const struct CapVec* self,
                   size_t *index,
                   long double* cm_min)
{
    struct Cap* cap=NULL;
    size_t i=0;
    double cm=0;

    *cm_min = 2.;
    for (i = 0; i < self->size; i++) {
        cap = &self->data[i];

        if (cap->cm >= 0.0) {
            cm = cap->cm;
        } else {
            cm = 2. + cap->cm;
        }

        if (cm <= *cm_min) {
            *index= i;
            *cm_min = cm;
        }
    }

}


/*

   CapForRand code

*/

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
