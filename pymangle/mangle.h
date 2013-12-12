#ifndef _MANGLE_MASK_H
#define _MANGLE_MASK_H

#include "defs.h"
#include "pixel.h"
#include "polygon.h"


struct MangleMask {
    int64 npoly;
    struct PolygonVec* poly_vec;

    double total_area;
    int64 pixelres;
    int64 maxpix;
    char pixeltype;
    struct PixelListVec* pixel_list_vec;

    int snapped;
    int balkanized;

    int verbose;

    char* filename;
    FILE* fptr;
    char buff[_MANGLE_SMALL_BUFFSIZE];


    // for error messages
    char err[_MANGLE_LARGE_BUFFSIZE];

};

struct MangleMask* mangle_new(void);
struct MangleMask* mangle_free(struct MangleMask* self);
void mangle_clear(struct MangleMask* self);
void mangle_set_verbosity(struct MangleMask* self, int verbosity);

void mangle_print(FILE* fptr, struct MangleMask* self, int verbosity);

int mangle_read(struct MangleMask* self, const char* filename);
int mangle_read_header(struct MangleMask* self);

// sum the areas for all polygons and store in ->total_area
void mangle_calc_area_and_maxpix(struct MangleMask* self);

int set_pixel_map(struct MangleMask* self);



/*
 * check the point against the mask.  If found, will return the
 * id and weight.  These default to -1 and 0
 *
 * this version does not use pixelization
 */
int mangle_polyid_and_weight_nopix(struct MangleMask *self, 
                                   struct Point *pt, 
                                   int64 *poly_id,
                                   double *weight);

/*
 * check the point against a pixelized mask.  If found, will return the id and
 * weight.  These default to -1 and 0
 */

int mangle_polyid_and_weight_pix(struct MangleMask *self, 
                                 struct Point *pt, 
                                 int64 *poly_id,
                                 double *weight);


/*
 * this chooses the right function based on pixeltype
 */

int mangle_polyid_and_weight(struct MangleMask *self, 
                             struct Point *pt, 
                             int64 *poly_id,
                             double *weight);

/*
 * inline version
 *
 * using some magic: leaving val at the end of this
 * block lets it become the value in an expression,
 */
#define MANGLE_POLYID_AND_WEIGHT(self, pt, poly_id, weight) ({            \
    int ret=0;                                                            \
    if ( (self)->pixeltype == 'u') {                                      \
        ret=mangle_polyid_and_weight_nopix(self,pt,poly_id,weight);       \
    } else {                                                              \
        ret=mangle_polyid_and_weight_pix(self,pt,poly_id,weight);         \
    }                                                                     \
    ret;                                                                  \
})

#endif
