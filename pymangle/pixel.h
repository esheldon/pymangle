#ifndef _MANGLE_PIXLIST_H
#define _MANGLE_PIXLIST_H

#include "mangle.h"
#include "point.h"
#include "stack.h"

struct PixelListVec {
    char pixeltype;
    int64 pixelres;

    // a vector of int64 stacks
    size_t size;
    struct i64stack** data;
};


struct PixelListVec* 
PixelListVec_new(size_t n);
struct PixelListVec* PixelListVec_free(struct PixelListVec* self);

// extract the pixel scheme and resolution from the input string
// which sould be [res][scheme] e.g. 9s
int pixel_parse_scheme(char buff[_MANGLE_SMALL_BUFFSIZE], 
                       int64* res, char* pixeltype);


int64 get_pixel_simple(int64 pixelres, struct Point* pt);



#endif

