#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mangle.h"
#include "pixel.h"
#include "stack.h"
#include "defs.h"

struct PixelListVec* 
PixelListVec_new(size_t n)
{
    struct PixelListVec* self=NULL;
    size_t i=0;

    if (n <= 0) {
        wlog("Vectors must be size > 0, got %ld", n);
        return NULL;
    }
    self=calloc(1, sizeof(struct PixelListVec));
    if (self == NULL) {
        wlog("Could not allocate pixel list vector");
        return NULL;
    }
    // array of pointers. The pointers will be NULL
    self->data = calloc(n, sizeof(struct i64stack*));
    if (self->data == NULL) {
        free(self);
        wlog("Could not allocate %ld pixel list pointers", n);
        return NULL;
    }

    for (i=0; i<n; i++) {
        self->data[i] = i64stack_new(0);
    }
    self->size=n;
    return self;
}

struct PixelListVec* 
PixelListVec_free(struct PixelListVec* self)
{
    size_t i=0;
    struct i64stack* s=NULL;
    if (self != NULL) {
	if (self->data != NULL) {
	    for (i=0; i<self->size; i++) {
		s = self->data[i];
		if (s != NULL) {
		    s=i64stack_delete(s);
		}
	    }
	    free(self->data);
	}
	
        free(self);
    }

    return self;
}




int pixel_parse_scheme(char buff[_MANGLE_SMALL_BUFFSIZE], 
                       int64* res, char* pixeltype) {
    int status=1;
    char pixres_buff[_MANGLE_SMALL_BUFFSIZE] = {0};
    char* ptr=NULL;
    ssize_t res_bytes=0;

    ptr = strchr(buff, 's');
    if (ptr == NULL) {
        status=0;
        wlog("Only support pix scheme s, got: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    *pixeltype = 's';

    // extract the numerical prefactor, which is the resolution
    res_bytes = (ptr-buff);
    if (res_bytes > 9) {
        status=0;
        wlog("pix scheme designation too big: '%s'", buff);
        goto _get_pix_scheme_errout;
    }
    strncpy(pixres_buff, buff, res_bytes);

    if (1 != sscanf(pixres_buff, "%ld", res)) {
        status=0;
        wlog("Could not extract resolution from pix scheme: '%s'", buff);
        goto _get_pix_scheme_errout;
    }

    // negative resolution is equivalent to 'u' or unpixelized
    if (*res < 0) {
        *pixeltype='u';
    }

_get_pix_scheme_errout:
    return status;
}

int64
get_pixel_simple(int64 pixelres, struct Point* pt)
{
    int64 pix=0;

    int64 i=0;
    int64 ps=0, p2=1;
    long double cth=0;
    int64 n=0, m=0;
    if (pixelres > 0) {
        for (i=0; i<pixelres; i++) { // Work out # pixels/dim and start pix.
            p2  = p2<<1;
            ps += (p2/2)*(p2/2);
        }
      cth = cosl(pt->theta);
      n   = (cth==1.0) ? 0: (int64) ( ceill( (1.0-cth)/2 * p2 )-1 );
      m   = (int64) ( floorl( (pt->phi/2./M_PI)*p2 ) );
      pix = p2*n+m + ps;

    }
    return pix;
}



