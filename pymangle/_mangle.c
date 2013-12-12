//#define NPY_NO_DEPRECATED_API

#include <string.h>
#include <math.h>
#include <Python.h>
#include "numpy/arrayobject.h" 
#include "mangle.h"
#include "point.h"
#include "cap.h"
#include "polygon.h"
#include "stack.h"
#include "pixel.h"
#include "rand.h"


struct PyMangleMask {
    PyObject_HEAD

    struct MangleMask* mask;
};



/*
 * Initalize the mangle mask.  Read the file and, if pixelized, 
 * set the pixel mask
 */

static int
PyMangleMask_init(struct PyMangleMask* self, PyObject *args, PyObject *kwds)
{
    char* filename=NULL;
    int verbose=0;
    if (!PyArg_ParseTuple(args, (char*)"si", &filename, &verbose)) {
        return -1;
    }

    self->mask = mangle_new();
    if (!self->mask) {
        PyErr_SetString(PyExc_MemoryError, "Error creating mangle mask struct");
        return -1;
    }
    mangle_set_verbosity(self->mask, verbose);
    if (!mangle_read(self->mask, filename)) {
        PyErr_Format(PyExc_IOError, "Error reading mangle mask %s",filename);
        return -1;
    }
    return 0;
}

/*
 * we use sprintf since PyString_FromFormat doesn't accept floating point types
 */

static PyObject *
PyMangleMask_repr(struct PyMangleMask* self) {
    npy_intp npoly;
    npy_intp npix;
    char buff[1024];
    struct MangleMask* mask=NULL;

    mask = self->mask;
    npoly = (mask->poly_vec != NULL) ? mask->poly_vec->size : 0;
    npix = (mask->pixel_list_vec != NULL) ? mask->pixel_list_vec->size : 0;

    sprintf(buff,
            "Mangle\n"
            "\tfile:       %s\n"
            "\tarea:       %g sqdeg\n"
            "\tnpoly:      %ld\n"
            "\tpixeltype:  '%c'\n"
            "\tpixelres:   %ld\n"
            "\tnpix:       %ld\n"
            "\tsnapped:    %d\n"
            "\tbalkanized: %d\n"
            "\tverbose:    %d\n", 
            mask->filename, mask->total_area*R2D*R2D, 
            npoly, mask->pixeltype, mask->pixelres, npix, 
            mask->snapped, mask->balkanized,
            mask->verbose);
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)buff);
#else
    return PyString_FromString((const char*)buff);
#endif
}

static PyObject *
PyMangleMask_area(struct PyMangleMask* self) {
    return PyFloat_FromDouble(self->mask->total_area*R2D*R2D);
}
static PyObject *
PyMangleMask_npoly(struct PyMangleMask* self) {
    return PyLong_FromLongLong( (PY_LONG_LONG) self->mask->npoly);
}


static PyObject *
PyMangleMask_is_pixelized(struct PyMangleMask* self) {
    if (self->mask->pixeltype != 'u') {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
static PyObject *
PyMangleMask_pixeltype(struct PyMangleMask* self) {
    char ptype[2];
    ptype[0] = self->mask->pixeltype;
    ptype[1] = '\0';
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*) ptype);
#else
    return PyString_FromString((const char* ) ptype);
#endif
}
static PyObject *
PyMangleMask_pixelres(struct PyMangleMask* self) {
    return PyLong_FromLongLong( (PY_LONG_LONG) self->mask->pixelres);
}
static PyObject *
PyMangleMask_maxpix(struct PyMangleMask* self) {
    return PyLong_FromLongLong( (PY_LONG_LONG) self->mask->maxpix);
}

static PyObject *
PyMangleMask_filename(struct PyMangleMask* self) {
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString( (const char* ) self->mask->filename);
#else
    return PyString_FromString( (const char* ) self->mask->filename);
#endif
}




static PyObject *
PyMangleMask_is_snapped(struct PyMangleMask* self) {
    if (self->mask->snapped) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}
static PyObject *
PyMangleMask_is_balkanized(struct PyMangleMask* self) {
    if (self->mask->balkanized) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}




static void
cleanup(struct PyMangleMask* self)
{
    if (self->mask->verbose > 2)
        fprintf(stderr,"mask struct\n");
    self->mask = mangle_free(self->mask);
}



static void
PyMangleMask_dealloc(struct PyMangleMask* self)
{

    cleanup(self);

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    // old way, removed in python 3
    self->ob_type->tp_free((PyObject*)self);
#endif
}




static PyObject*
make_intp_array(npy_intp size, const char* name, npy_intp** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_INTP, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
}

static PyObject*
make_double_array(npy_intp size, const char* name, double** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_FLOAT64, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
}
static double* 
check_double_array(PyObject* array, const char* name, npy_intp* size)
{
    double* ptr=NULL;
    if (!PyArray_Check(array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type 64-bit float",name);
        return NULL;
    }
    if (NPY_DOUBLE != PyArray_TYPE((PyArrayObject*)array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type 64-bit float",name);
        return NULL;
    }

    ptr = PyArray_DATA((PyArrayObject*)array);
    *size = PyArray_SIZE((PyArrayObject*)array);

    return ptr;
}

static int
check_ra_dec_arrays(PyObject* ra_obj, PyObject* dec_obj,
                    double** ra_ptr, npy_intp* nra, 
                    double** dec_ptr, npy_intp*ndec)
{
    if (!(*ra_ptr=check_double_array(ra_obj,"ra",nra)))
        return 0;
    if (!(*dec_ptr=check_double_array(dec_obj,"dec",ndec)))
        return 0;
    if (*nra != *ndec) {
        PyErr_Format(PyExc_ValueError,
                "ra,dec must same length, got (%ld,%ld)",*nra,*ndec);
        return 0;
    }

    return 1;
}

/*
 * check ra,dec points, returning both poly_id and weight
 * in a tuple
 */

static PyObject*
PyMangleMask_polyid_and_weight(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* poly_id_obj=NULL;
    PyObject* weight_obj=NULL;
    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double* weight_ptr=NULL;
    npy_intp* poly_id_ptr=NULL;
    npy_intp nra=0, ndec=0, i=0;

    PyObject* tuple=NULL;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }

    if (!(poly_id_obj=make_intp_array(nra, "polyid", &poly_id_ptr))) {
        status=0;
        goto _poly_id_and_weight_cleanup;
    }
    if (!(weight_obj=make_double_array(nra, "weight", &weight_ptr))) {
        status=0;
        goto _poly_id_and_weight_cleanup;
    }

    for (i=0; i<nra; i++) {
        point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

        status=mangle_polyid_and_weight_nopix(self->mask, 
                                              &pt, 
                                              poly_id_ptr, 
                                              weight_ptr);

        if (status != 1) {
            goto _poly_id_and_weight_cleanup;
        }
        ra_ptr++;
        dec_ptr++;
        poly_id_ptr++;
        weight_ptr++;
    }

_poly_id_and_weight_cleanup:
    if (status != 1) {
        Py_XDECREF(poly_id_obj);
        Py_XDECREF(weight_obj);
        Py_XDECREF(tuple);
        return NULL;
    }

    tuple=PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, poly_id_obj);
    PyTuple_SetItem(tuple, 1, weight_obj);
    return tuple;
}

/*
 * check ra,dec points, returning the polyid
 */

static PyObject*
PyMangleMask_polyid(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* poly_id_obj=NULL;

    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double weight=0;
    npy_intp* poly_id_ptr=NULL;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(poly_id_obj=make_intp_array(nra, "polyid", &poly_id_ptr))) {
        return NULL;
    }

    for (i=0; i<nra; i++) {
        point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

        //status=mangle_polyid_and_weight(self->mask, 
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        poly_id_ptr, 
                                        &weight);

        if (status != 1) {
            goto _poly_id_cleanup;
        }
        ra_ptr++;
        dec_ptr++;
        poly_id_ptr++;
    }

_poly_id_cleanup:
    if (status != 1) {
        Py_XDECREF(poly_id_obj);
        return NULL;
    }
    return poly_id_obj;
}

/*
 * check ra,dec points, returning weight
 */

static PyObject*
PyMangleMask_weight(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* weight_obj=NULL;
    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double* weight_ptr=NULL;
    npy_intp poly_id=0;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(weight_obj=make_double_array(nra, "weight", &weight_ptr))) {
        return NULL;
    }
    for (i=0; i<nra; i++) {
        point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

        //status=mangle_polyid_and_weight(self->mask, 
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        &poly_id, 
                                        weight_ptr);

        if (status != 1) {
            goto _weight_cleanup;
        }
        ra_ptr++;
        dec_ptr++;
        weight_ptr++;
    }

_weight_cleanup:
    if (status != 1) {
        Py_XDECREF(weight_obj);
        return NULL;
    }
    return weight_obj;
}

/*
 * check if ra,dec points are contained
 */

static PyObject*
PyMangleMask_contains(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* contained_obj=NULL;
    npy_intp* cont_ptr=NULL;
    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double weight=0;
    npy_intp poly_id=0;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(contained_obj=make_intp_array(nra, "contained", &cont_ptr))) {
        return NULL;
    }

    for (i=0; i<nra; i++) {
        point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

        //status=mangle_polyid_and_weight(self->mask, 
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        &poly_id, 
                                        &weight);

        if (status != 1) {
            goto _weight_cleanup;
        }

        if (poly_id >= 0) {
            *cont_ptr = 1;
        }
        ra_ptr++;
        dec_ptr++;
        cont_ptr++;
    }

_weight_cleanup:
    if (status != 1) {
        Py_XDECREF(contained_obj);
        return NULL;
    }
    return contained_obj;
}


/*
 * Generate random points.  This function draws randoms initially from the full
 * sky, so can be inefficient for small masks.
 *
 * The reason to use this function is it does not require any knowledge of the
 * domain covered by the mask.
 */

static PyObject*
PyMangleMask_genrand(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    PY_LONG_LONG nrand=0;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* tuple=NULL;
    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double weight=0;
    npy_intp poly_id=0;
    npy_intp ngood=0;
    double theta=0, phi=0;


    if (!PyArg_ParseTuple(args, (char*)"L", &nrand)) {
        return NULL;
    }

    if (nrand <= 0) {
        PyErr_Format(PyExc_ValueError, 
                "nrand should be > 0, got (%ld)",(npy_intp)nrand);
        status=0;
        goto _genrand_cleanup;
    }

    if (!(ra_obj=make_double_array(nrand, "ra", &ra_ptr))) {
        status=0;
        goto _genrand_cleanup;
    }
    if (!(dec_obj=make_double_array(nrand, "dec", &dec_ptr))) {
        status=0;
        goto _genrand_cleanup;
    }

    seed_random();

    while (ngood < nrand) {
        genrand_theta_phi_allsky(&theta, &phi);
        point_set_from_thetaphi(&pt, theta, phi);

        //status=mangle_polyid_and_weight(self->mask, 
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        &poly_id, 
                                        &weight);

        if (status != 1) {
            goto _genrand_cleanup;
        }

        if (poly_id >= 0) {
            // rely on short circuiting
            if (weight < 1.0 || drand48() < weight) {
                ngood++;
                radec_from_point(&pt, ra_ptr, dec_ptr);
                ra_ptr++;
                dec_ptr++;
            }
        }
    }

_genrand_cleanup:
    if (status != 1) {
        Py_XDECREF(ra_obj);
        Py_XDECREF(dec_obj);
        Py_XDECREF(tuple);
        return NULL;
    }

    tuple=PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, ra_obj);
    PyTuple_SetItem(tuple, 1, dec_obj);
    return tuple;
}



/*
 * Generate random points in the input ra,dec range.
 *
 * Use this if you have a small mask, as choosing the range wisely can save a
 * lot of time.  But be careful: if you choose your range poorly, it may never
 * find the points!
 */

static PyObject*
PyMangleMask_genrand_range(struct PyMangleMask* self, PyObject* args)
{
    int status=1;
    PY_LONG_LONG nrand=0;
    double ramin=0,ramax=0,decmin=0,decmax=0;
    double cthmin=0,cthmax=0,phimin=0,phimax=0;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* tuple=NULL;
    double* ra_ptr=NULL;
    double* dec_ptr=NULL;
    double weight=0;
    npy_intp poly_id=0;
    npy_intp ngood=0;
    double theta=0, phi=0;


    if (!PyArg_ParseTuple(args, (char*)"Ldddd", 
                          &nrand, &ramin, &ramax, &decmin, &decmax)) {
        return NULL;
    }

    if (nrand <= 0) {
        PyErr_Format(PyExc_ValueError, 
                "nrand should be > 0, got (%ld)",(npy_intp)nrand);
        status=0;
        goto _genrand_range_cleanup;
    }
    if (!radec_range_to_costhetaphi(ramin,ramax,decmin,decmax,
                                    &cthmin,&cthmax,&phimin,&phimax)) {
        status=0;
        goto _genrand_range_cleanup;
    }

    if (!(ra_obj=make_double_array(nrand, "ra", &ra_ptr))) {
        status=0;
        goto _genrand_range_cleanup;
    }
    if (!(dec_obj=make_double_array(nrand, "dec", &dec_ptr))) {
        status=0;
        goto _genrand_range_cleanup;
    }

    seed_random();

    while (ngood < nrand) {
        genrand_theta_phi(cthmin,cthmax,phimin,phimax,&theta, &phi);
        point_set_from_thetaphi(&pt, theta, phi);

        //status=mangle_polyid_and_weight(self->mask, &pt, &poly_id, &weight);
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, &pt, &poly_id, &weight);

        if (status != 1) {
            goto _genrand_range_cleanup;
        }

        if (poly_id >= 0) {
            // rely on short circuiting
            if (weight < 1.0 || drand48() < weight) {
                ngood++;
                radec_from_point(&pt, ra_ptr, dec_ptr);
                ra_ptr++;
                dec_ptr++;
            }
        }
    }

_genrand_range_cleanup:
    if (status != 1) {
        Py_XDECREF(ra_obj);
        Py_XDECREF(dec_obj);
        Py_XDECREF(tuple);
        return NULL;
    }

    tuple=PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, ra_obj);
    PyTuple_SetItem(tuple, 1, dec_obj);
    return tuple;
}



static PyMethodDef PyMangleMask_methods[] = {
    {"polyid_and_weight", (PyCFunction)PyMangleMask_polyid_and_weight, METH_VARARGS, 
        "polyid_and_weight(ra,dec)\n"
        "\n"
        "Check points against mask, returning (poly_id,weight).\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f8'\n"
        "dec: array\n"
        "    A numpy array of type 'f8'\n"},
    {"polyid",            (PyCFunction)PyMangleMask_polyid,            METH_VARARGS, 
        "polyid(ra,dec)\n"
        "\n"
        "Check points against mask, returning the polygon id or -1.\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f8'\n"
        "dec: array\n"
        "    A numpy array of type 'f8'\n"},
    {"weight",            (PyCFunction)PyMangleMask_weight,            METH_VARARGS, 
        "weight(ra,dec)\n"
        "\n"
        "Check points against mask, returning the weight or 0.0\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f8'\n"
        "dec: array\n"
        "    A numpy array of type 'f8'\n"},
    {"contains",          (PyCFunction)PyMangleMask_contains,          METH_VARARGS, 
        "contains(ra,dec)\n"
        "\n"
        "Check points against mask, returning 1 if contained 0 if not\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f8'\n"
        "dec: array\n"
        "    A numpy array of type 'f8'\n"},
    {"genrand",           (PyCFunction)PyMangleMask_genrand,           METH_VARARGS, 
        "genrand(nrand)\n"
        "\n"
        "Generate random points that are within the mask.\n"
        "\n"
        "parameters\n"
        "----------\n"
        "nrand: number\n"
        "    The number of random points to generate\n"},
    {"genrand_range",     (PyCFunction)PyMangleMask_genrand_range,     METH_VARARGS, 
        "genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "\n"
        "Generate random points inside the input range and the mask.\n"
        "\n"
        "parameters\n"
        "----------\n"
        "nrand: number\n"
        "    The number of random points to generate\n"
        "ramin: double\n"
        "    The minimum ra\n"
        "ramax: double\n"
        "    The maximum ra\n"
        "decmin: double\n"
        "    The minimum dec\n"
        "decmax: double\n"
        "    The maximum dec\n"},

    {"get_filename",       (PyCFunction)PyMangleMask_filename,         METH_VARARGS, 
        "filename()\n"
        "\n"
        "Return the mask filename.\n"},



    {"get_area",       (PyCFunction)PyMangleMask_area,         METH_VARARGS, 
        "area()\n"
        "\n"
        "Return the area within mask in square degrees.\n"},

    {"get_npoly",       (PyCFunction)PyMangleMask_npoly,         METH_VARARGS, 
        "npoly()\n"
        "\n"
        "Return the polygon count.\n"},

    {"get_is_pixelized",       (PyCFunction)PyMangleMask_is_pixelized,      METH_VARARGS, 
        "get_is_pixelized()\n"
        "\n"
        "True if pixelized.\n"},
    {"get_maxpix",       (PyCFunction)PyMangleMask_maxpix,         METH_VARARGS, 
        "maxpix()\n"
        "\n"
        "Return the maximum pixel id.\n"},
    {"get_pixelres",       (PyCFunction)PyMangleMask_pixelres,         METH_VARARGS, 
        "pixelres()\n"
        "\n"
        "Return the pixel resolution, -1 if unpixelized.\n"},
    {"get_pixeltype",       (PyCFunction)PyMangleMask_pixeltype,         METH_VARARGS, 
        "pixeltype()\n"
        "\n"
        "Return the pixel type as a string. 'u' if unpixelized.\n"},



    {"get_is_snapped",       (PyCFunction)PyMangleMask_is_snapped,         METH_VARARGS, 
        "get_is_snapped()\n"
        "\n"
        "Return True if the snapped keyword was found in the header.\n"},
    {"get_is_balkanized",       (PyCFunction)PyMangleMask_is_balkanized,         METH_VARARGS, 
        "get_is_balkanized()\n"
        "\n"
        "Return True if the balkanized keyword was found in the header.\n"},

    {NULL}  /* Sentinel */
};


static PyTypeObject PyMangleMaskType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_mangle.Mangle",             /*tp_name*/
    sizeof(struct PyMangleMask), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyMangleMask_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyMangleMask_repr,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A class to work with Mangle masks.\n"
        "\n"
        "construction\n"
        "    import mangle\n"
        "    m=mangle.Mangle(mask_file, verbose=False)\n"
        "\n"
        "\n"
        "read-only properties\n"
        "--------------------\n"
        "filename\n"
        "area\n"
        "npoly\n"
        "is_pixelized\n"
        "pixeltype\n"
        "pixelres\n"
        "maxpix\n"
        "is_snapped\n"
        "is_balkanized\n"
        "\n"
        "See docs for each property for more details.\n"
        "\n"
        "methods\n"
        "-------\n"
        "polyid(ra,dec)\n"
        "weight(ra,dec)\n"
        "polyid_and_weight(ra,dec)\n"
        "contains(ra,dec)\n"
        "genrand(nrand)\n"
        "genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "\n"
        "getters (correspond to properties above)\n"
        "----------------------------------------\n"
        "get_filename()\n"
        "get_area()\n"
        "get_npoly()\n"
        "get_is_pixelized()\n"
        "get_pixeltype()\n"
        "get_pixelres()\n"
        "get_maxpix()\n"
        "get_is_snapped()\n"
        "get_is_balkanized()\n"
        "\n"
        "See docs for each method for more detail.\n",
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyMangleMask_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyMangleMask_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};

static PyMethodDef mangle_methods[] = {
    {NULL}  /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_mangle",      /* m_name */
        "Defines the Mangle class and some methods",  /* m_doc */
        -1,                  /* m_size */
        mangle_methods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit__mangle(void) 
#else
init_mangle(void) 
#endif
{
    PyObject* m;


    PyMangleMaskType.tp_new = PyType_GenericNew;

#if PY_MAJOR_VERSION >= 3
    if (PyType_Ready(&PyMangleMaskType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&moduledef);
    if (m==NULL) {
        return NULL;
    }

#else
    if (PyType_Ready(&PyMangleMaskType) < 0) {
        return;
    }
    m = Py_InitModule3("_mangle", mangle_methods, 
            "This module defines a class to work with Mangle masks.\n"
            "\n"
            "construction\n"
            "    import mangle\n"
            "    m=mangle.Mangle(mask_file, verbose=False)\n"
            "\n"
            "methods\n"
            "-------\n"
            "polyid(ra,dec)\n"
            "weight(ra,dec)\n"
            "polyid_and_weight(ra,dec)\n"
            "contains(ra,dec)\n"
            "genrand(nrand)\n"
            "genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
            "is_snapped()\n"
            "is_balkanized()\n"
            "\n"
            "See docs for each method for more detailed info\n");
    if (m==NULL) {
        return;
    }
#endif

    Py_INCREF(&PyMangleMaskType);
    PyModule_AddObject(m, "Mangle", (PyObject *)&PyMangleMaskType);

    import_array();
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
