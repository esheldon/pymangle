#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

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

/*
   Cap class
*/
struct PyMangleCap {
    PyObject_HEAD

    struct Cap cap;
};

/*

  Initalize the mangle cap.

  In order to support long double, the input is a numpy array of length 4
  and type numpy.longdouble.  We cannot pass scalars because PyArg_ParseTuple
  only supports double

  we are assuming this longdouble corresponds to c long double

  No error checking is performed here, do that in python
 */

static int
PyMangleCap_init(struct PyMangleCap* self, PyObject *args, PyObject *kwds)
{
    return 0;
}

static PyObject*
PyMangleCap_set(struct PyMangleCap* self, PyObject *args, PyObject *kwds)
{
    PyObject* arr_obj=NULL;
    long double *data=NULL;

    if (!PyArg_ParseTuple(args, (char*)"O", &arr_obj)) {
        return NULL;
    }

    data = (long double *) PyArray_DATA( (PyArrayObject*) arr_obj);

    cap_set(&self->cap, data[0], data[1], data[2], data[3]);

    Py_RETURN_NONE;
}


static void
PyMangleCap_dealloc(struct PyMangleCap* self)
{

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    // old way, removed in python 3
    self->ob_type->tp_free((PyObject*)self);
#endif

}


static PyObject *
PyMangleCap_repr(struct PyMangleCap* self) {
    char buff[128];
    struct Cap* cap=&self->cap;

    snprint_cap(cap, buff, sizeof(buff));

#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)buff);
#else
    return PyString_FromString((const char*)buff);
#endif
}

// methods for PyMangleCap
static PyMethodDef PyMangleCap_methods[] = { 
    {"set", (PyCFunction)PyMangleCap_set, METH_VARARGS, "set the cap data\n"},
    {NULL}
};

// the type definition for PyMangleCap

static PyTypeObject PyMangleCapType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_mangle.Mangle",             /*tp_name*/
    sizeof(struct PyMangleCap), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyMangleCap_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyMangleCap_repr,                         /*tp_repr*/
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
        "    import pymangle\n"
        "    m=pymangle.Mangle(mask_file, verbose=False)\n"
        "\n"
        "\n"
        "read-only properties\n"
        "--------------------\n"
        "    filename\n"
        "    area\n"
        "    npoly\n"
        "    is_pixelized\n"
        "    pixeltype\n"
        "    pixelres\n"
        "    maxpix\n"
        "    is_snapped\n"
        "    is_balkanized\n"
        "    weightfile\n"
        "\n"
        "See docs for each property for more details.\n"
        "\n"
        "methods\n"
        "----------------------\n"
        "    polyid(ra,dec)\n"
        "    weight(ra,dec)\n"
        "    polyid_and_weight(ra,dec)\n"
        "    contains(ra,dec)\n"
        "    genrand(nrand)\n"
        "    genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "    calc_simplepix(ra,dec)\n"
        "    read_weights(weightfile)\n"
        "\n"
        "getters (correspond to properties above)\n"
        "----------------------------------------\n"
        "    get_filename()\n"
        "    get_weightfile()\n"
        "    get_area()\n"
        "    get_npoly()\n"
        "    get_is_pixelized()\n"
        "    get_pixeltype()\n"
        "    get_pixelres()\n"
        "    get_maxpix()\n"
        "    get_is_snapped()\n"
        "    get_is_balkanized()\n"
        "    get_pixels()\n"
        "    get_weights()\n"
        "    get_areas()\n"
        "\n"
        "setter (corresponding to property above)\n"
        "----------------------------------------\n"
        "    set_weights(weights)\n"
        "\n"
        "See docs for each method for more detail.\n",
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyMangleCap_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyMangleCap_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};



/*

   CapVec class

*/

struct PyMangleCapVec {
    PyObject_HEAD

    struct CapVec* caps;
};

static int
PyMangleCapVec_init(struct PyMangleCapVec* self, PyObject *args, PyObject *kwds)
{
    Py_ssize_t n=0;

    if (!PyArg_ParseTuple(args, (char*)"n", &n)) {
        return -1;
    }

    self->caps = capvec_zeros( (size_t) n);
    if (self->caps == NULL) {
        PyErr_SetString(PyExc_MemoryError, "out of memory allocating CapVec");
        return -1;
    }

    return 0;
}

static PyObject *
PyMangleCapVec_repr(struct PyMangleCapVec* self) {
    char buff[64];

    snprintf(buff,64, "MangleCapVec, ncaps: %lu", self->caps->size);

#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)buff);
#else
    return PyString_FromString((const char*)buff);
#endif
}


static void
PyMangleCapVec_dealloc(struct PyMangleCapVec* self)
{

    self->caps=capvec_free(self->caps);

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    // old way, removed in python 3
    self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject*
PyMangleCapVec_size(struct PyMangleCapVec* self)
{
    return Py_BuildValue("n",(Py_ssize_t) self->caps->size);
}



static PyObject*
PyMangleCapVec_set_cap(struct PyMangleCapVec* self, PyObject *args, PyObject *kwds)
{

    PyObject* cap_pyobj=NULL;
    struct PyMangleCap *cap_obj=NULL;

    Py_ssize_t index=0;

    if (!PyArg_ParseTuple(args, (char*)"nO", &index, &cap_pyobj)) {
        return NULL;
    }

    cap_obj = (struct PyMangleCap *) cap_pyobj;

    cap_set(&self->caps->data[index],
            cap_obj->cap.x,
            cap_obj->cap.y,
            cap_obj->cap.z,
            cap_obj->cap.cm);

    Py_RETURN_NONE;
}

static PyObject*
PyMangleCapVec_get_cap(struct PyMangleCapVec* self, PyObject *args, PyObject *kwds)
{
    PyTypeObject *type=&PyMangleCapType;

    const struct Cap* cap=NULL;
    PyObject* cap_copy=NULL;
    struct PyMangleCap* cap_st=NULL;

    Py_ssize_t index=0;

    if (!PyArg_ParseTuple(args, (char*)"n", &index)) {
        return NULL;
    }


    cap = &self->caps->data[index];

    cap_copy = _PyObject_New(type);
    cap_st=(struct PyMangleCap *) cap_copy;
    
    cap_st->cap.x = cap->x;
    cap_st->cap.y = cap->y;
    cap_st->cap.z = cap->z;
    cap_st->cap.cm = cap->cm;

    return cap_copy;
}





// methods for PyMangleCapVec
static PyMethodDef PyMangleCapVec_methods[] = { 
    {"size", (PyCFunction)PyMangleCapVec_size, METH_VARARGS, "get length of CapVec\n"},
    {"_set_cap", (PyCFunction)PyMangleCapVec_set_cap, METH_VARARGS, "set the cap data at the specified index\n"},
    {"_get_cap", (PyCFunction)PyMangleCapVec_get_cap, METH_VARARGS, "get a copy of the cap at the specified index\n"},
    {NULL}
};

// the type definition for PyMangleCapVec

static PyTypeObject PyMangleCapVecType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_mangle.Mangle",             /*tp_name*/
    sizeof(struct PyMangleCapVec), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyMangleCapVec_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyMangleCapVec_repr,                         /*tp_repr*/
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
        "    import pymangle\n"
        "    m=pymangle.Mangle(mask_file, verbose=False)\n"
        "\n"
        "\n"
        "read-only properties\n"
        "--------------------\n"
        "    filename\n"
        "    area\n"
        "    npoly\n"
        "    is_pixelized\n"
        "    pixeltype\n"
        "    pixelres\n"
        "    maxpix\n"
        "    is_snapped\n"
        "    is_balkanized\n"
        "    weightfile\n"
        "\n"
        "See docs for each property for more details.\n"
        "\n"
        "methods\n"
        "----------------------\n"
        "    polyid(ra,dec)\n"
        "    weight(ra,dec)\n"
        "    polyid_and_weight(ra,dec)\n"
        "    contains(ra,dec)\n"
        "    genrand(nrand)\n"
        "    genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "    calc_simplepix(ra,dec)\n"
        "    read_weights(weightfile)\n"
        "\n"
        "getters (correspond to properties above)\n"
        "----------------------------------------\n"
        "    get_filename()\n"
        "    get_weightfile()\n"
        "    get_area()\n"
        "    get_npoly()\n"
        "    get_is_pixelized()\n"
        "    get_pixeltype()\n"
        "    get_pixelres()\n"
        "    get_maxpix()\n"
        "    get_is_snapped()\n"
        "    get_is_balkanized()\n"
        "    get_pixels()\n"
        "    get_weights()\n"
        "    get_areas()\n"
        "\n"
        "setter (corresponding to property above)\n"
        "----------------------------------------\n"
        "    set_weights(weights)\n"
        "\n"
        "See docs for each method for more detail.\n",
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyMangleCapVec_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyMangleCapVec_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};



/* 
   Polygon class
*/

struct PyManglePolygon {
    PyObject_HEAD

    struct Polygon poly;
};

/*
   Initialize from a CapVec

   The data CapVec are copied
*/

static int
PyManglePolygon_init(struct PyManglePolygon* self, PyObject *args, PyObject *kwds)
{
    int64 poly_id=0, pixel_id=0;
    PyObject* wt_arr_obj=NULL;
    long double *wt_data=NULL;

    PyObject *caps_obj=NULL;
    struct PyMangleCapVec *caps_st=NULL;

    if (!PyArg_ParseTuple(args, (char*)"llOO", &poly_id, &pixel_id, &wt_arr_obj, &caps_obj)) {
        return -1;
    }

    wt_data = (long double *) PyArray_DATA( (PyArrayObject*) wt_arr_obj);

    caps_st=(struct PyMangleCapVec *) caps_obj;

    self->poly.poly_id  = poly_id;
    self->poly.pixel_id = pixel_id;
    self->poly.weight   = wt_data[0];

    // we need garea to calculate this
    self->poly.area = -1.0;

    self->poly.caps = capvec_copy(caps_st->caps);
    if (self->poly.caps == NULL) {
        PyErr_SetString(PyExc_MemoryError, "out of memory allocating CapVec");
        return -1;
    }

    return 0;
}

static void
PyManglePolygon_dealloc(struct PyManglePolygon* self)
{

    self->poly.caps=capvec_free(self->poly.caps);

#if ((PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 6) || (PY_MAJOR_VERSION == 3))
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    // old way, removed in python 3
    self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject *
PyManglePolygon_repr(struct PyManglePolygon* self) {
    char buff[64];

    snprintf(buff,
             sizeof(buff),
             "poly_id: %ld pixel_id: %ld weight: %.18Lg area: %.18Lg ncaps: %lu",
             self->poly.poly_id,
             self->poly.pixel_id,
             self->poly.weight,
             self->poly.area,
             self->poly.caps->size);

#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)buff);
#else
    return PyString_FromString((const char*)buff);
#endif
}


static PyObject*
PyManglePolygon_size(struct PyManglePolygon* self)
{
    return Py_BuildValue("n",(Py_ssize_t) self->poly.caps->size);
}

static PyObject*
PyManglePolygon_get_cap(struct PyManglePolygon* self, PyObject *args, PyObject *kwds)
{
    PyTypeObject *type=&PyMangleCapType;

    const struct Cap* cap=NULL;
    PyObject* cap_copy=NULL;
    struct PyMangleCap* cap_st=NULL;

    Py_ssize_t index=0;

    if (!PyArg_ParseTuple(args, (char*)"n", &index)) {
        return NULL;
    }


    cap = &self->poly.caps->data[index];

    cap_copy = _PyObject_New(type);
    cap_st=(struct PyMangleCap *) cap_copy;
    
    cap_st->cap.x = cap->x;
    cap_st->cap.y = cap->y;
    cap_st->cap.z = cap->z;
    cap_st->cap.cm = cap->cm;

    return cap_copy;
}




// methods for PyManglePolygon
static PyMethodDef PyManglePolygon_methods[] = { 
    {"size", (PyCFunction)PyManglePolygon_size, METH_VARARGS, "get length of Polygon\n"},
    {"_get_cap", (PyCFunction)PyManglePolygon_get_cap, METH_VARARGS, "get a copy of the cap at the specified index\n"},
    {NULL}
};

// the type definition for PyManglePolygon

static PyTypeObject PyManglePolygonType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
#endif
    "_mangle.Mangle",             /*tp_name*/
    sizeof(struct PyManglePolygon), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyManglePolygon_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    //0,                         /*tp_repr*/
    (reprfunc)PyManglePolygon_repr,                         /*tp_repr*/
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
        "    import pymangle\n"
        "    m=pymangle.Mangle(mask_file, verbose=False)\n"
        "\n"
        "\n"
        "read-only properties\n"
        "--------------------\n"
        "    filename\n"
        "    area\n"
        "    npoly\n"
        "    is_pixelized\n"
        "    pixeltype\n"
        "    pixelres\n"
        "    maxpix\n"
        "    is_snapped\n"
        "    is_balkanized\n"
        "    weightfile\n"
        "\n"
        "See docs for each property for more details.\n"
        "\n"
        "methods\n"
        "----------------------\n"
        "    polyid(ra,dec)\n"
        "    weight(ra,dec)\n"
        "    polyid_and_weight(ra,dec)\n"
        "    contains(ra,dec)\n"
        "    genrand(nrand)\n"
        "    genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "    calc_simplepix(ra,dec)\n"
        "    read_weights(weightfile)\n"
        "\n"
        "getters (correspond to properties above)\n"
        "----------------------------------------\n"
        "    get_filename()\n"
        "    get_weightfile()\n"
        "    get_area()\n"
        "    get_npoly()\n"
        "    get_is_pixelized()\n"
        "    get_pixeltype()\n"
        "    get_pixelres()\n"
        "    get_maxpix()\n"
        "    get_is_snapped()\n"
        "    get_is_balkanized()\n"
        "    get_pixels()\n"
        "    get_weights()\n"
        "    get_areas()\n"
        "\n"
        "setter (corresponding to property above)\n"
        "----------------------------------------\n"
        "    set_weights(weights)\n"
        "\n"
        "See docs for each method for more detail.\n",
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    PyManglePolygon_methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    //0,     /* tp_init */
    (initproc)PyManglePolygon_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                 /* tp_new */
};




/* 

   overall MangleMask class
   
*/


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

static PyObject *
PyMangleMask_read_weights(struct PyMangleMask* self, PyObject *args, PyObject *kwds)
{
    char *weightfile = NULL;

    if (!PyArg_ParseTuple(args, (char*)"s", &weightfile)) {
	Py_RETURN_FALSE;
    }

    if (!mangle_read_weights(self->mask, weightfile)) {
	PyErr_Format(PyExc_IOError,"Error reading weight file %s",weightfile);
	Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static PyObject *
PyMangleMask_set_weights(struct PyMangleMask* self, PyObject *args, PyObject *kwds)
{
    PyObject *weight_obj = NULL;
    long double *weights;

    if (!PyArg_ParseTuple(args, (char*)"O", &weight_obj)) {
	PyErr_SetString(PyExc_TypeError,"Failed to parse args to set_weights");
	Py_RETURN_FALSE;
    }

    if (PyArray_NDIM((PyArrayObject *)weight_obj) != 1) {
	PyErr_SetString(PyExc_ValueError,"Input to set_weights must be 1D array");
	Py_RETURN_FALSE;
    }

    if (PyArray_DIM((PyArrayObject *)weight_obj, 0) != self->mask->npoly) {
	PyErr_SetString(PyExc_ValueError,"Input number of weights must be equal to number of polygons.");
	Py_RETURN_FALSE;
    }

    weights = (long double *) PyArray_DATA((PyArrayObject *)weight_obj);

    if (!mangle_set_weights(self->mask, weights)) {
	PyErr_SetString(PyExc_ValueError,"Error setting weights");
	Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

/*
 * we use sprintf since PyString_FromFormat doesn't accept floating point types
 */

static PyObject *
PyMangleMask_repr(struct PyMangleMask* self) {
    npy_intp npoly;
    npy_intp npix;
    char buff[4096];
    struct MangleMask* mask=NULL;

    mask = self->mask;
    npoly = (mask->poly_vec != NULL) ? mask->poly_vec->size : 0;
    npix = (mask->pixel_list_vec != NULL) ? mask->pixel_list_vec->size : 0;
   
    snprintf(buff,4096,
	     "Mangle\n"
	     "\tfile:       %s\n"
	     "\tarea:       %Lg sqdeg\n"
	     "\tnpoly:      %ld\n"
	     "\tpixeltype:  '%c'\n"
	     "\tpixelres:   %ld\n"
	     "\treal:       %d\n"
	     "\tnpix:       %ld\n"
	     "\tsnapped:    %d\n"
	     "\tbalkanized: %d\n"
	     "\tweightfile: %s\n"
	     "\tverbose:    %d\n", 
	     mask->filename, mask->total_area*R2D*R2D, 
	     npoly, mask->pixeltype, mask->pixelres, mask->real, npix, 
	     mask->snapped, mask->balkanized, mask->weightfile,
	     mask->verbose);
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString((const char*)buff);
#else
    return PyString_FromString((const char*)buff);
#endif
}

static PyObject *
PyMangleMask_area(struct PyMangleMask* self) {
    PyObject *longdouble_obj = NULL;
    npy_intp dims[1] = {1};
    long double total_area_deg2;

    longdouble_obj = PyArray_ZEROS(0,dims,NPY_LONGDOUBLE,0);
    total_area_deg2 = self->mask->total_area*R2D*R2D;
    memcpy(PyArray_DATA((PyArrayObject *)longdouble_obj), &total_area_deg2, sizeof(long double));
    return PyArray_Return((PyArrayObject *)longdouble_obj);
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
PyMangleMask_weightfile(struct PyMangleMask* self) {

    
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString( (const char* ) self->mask->weightfile);
#else
    return PyString_FromString( (const char* ) self->mask->weightfile);
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
make_bool_array(npy_intp size, const char* name, npy_bool** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_BOOL, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
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
make_longdouble_array(npy_intp size, const char* name, long double** ptr)
{
    PyObject* array=NULL;
    npy_intp dims[1];
    int ndims=1;
    if (size <= 0) {
        PyErr_Format(PyExc_ValueError, "size of %s array must be > 0",name);
        return NULL;
    }

    dims[0] = size;
    array = PyArray_ZEROS(ndims, dims, NPY_LONGDOUBLE, 0);
    if (array==NULL) {
        PyErr_Format(PyExc_MemoryError, "could not create %s array",name);
        return NULL;
    }

    *ptr = PyArray_DATA((PyArrayObject*)array);
    return array;
}
static long double* 
check_longdouble_array(PyObject* array, const char* name, npy_intp* size)
{
    long double* ptr=NULL;
    if (!PyArray_Check(array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type 64-bit float",name);
        return NULL;
    }
    if (NPY_LONGDOUBLE != PyArray_TYPE((PyArrayObject*)array)) {
        PyErr_Format(PyExc_ValueError,
                "%s must be a numpy array of type '128-bit' float (long double)",name);
        return NULL;
    }

    ptr = PyArray_DATA((PyArrayObject*)array);
    *size = PyArray_SIZE((PyArrayObject*)array);

    return ptr;
}

static int
check_ra_dec_arrays(PyObject* ra_obj, PyObject* dec_obj,
                    long double** ra_ptr, npy_intp* nra, 
                    long double** dec_ptr, npy_intp*ndec)
{
    if (!(*ra_ptr=check_longdouble_array(ra_obj,"ra",nra)))
        return 0;
    if (!(*dec_ptr=check_longdouble_array(dec_obj,"dec",ndec)))
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
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double* weight_ptr=NULL;
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
    if (!(weight_obj=make_longdouble_array(nra, "weight", &weight_ptr))) {
        status=0;
        goto _poly_id_and_weight_cleanup;
    }

    for (i=0; i<nra; i++) {
        point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

	//status=mangle_polyid_and_weight_nopix(self->mask,
	status=MANGLE_POLYID_AND_WEIGHT(self->mask, 		  
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

    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double weight=0;
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
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double* weight_ptr=NULL;
    npy_intp poly_id=0;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(weight_obj=make_longdouble_array(nra, "weight", &weight_ptr))) {
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
    npy_bool* cont_ptr=NULL;
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double weight=0;
    npy_intp poly_id=0;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(contained_obj=make_bool_array(nra, "contained", &cont_ptr))) {
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
   check the quadrants in the specified cap against the mask
   using a monte-carlo approach

   The quadrant is considered "good" if the fraction of masked
   points is less than pmax

   parameters
   ----------
   ra
       center of cap in degrees
   dec
       center of cap in degrees
   angle_degrees
       the opening angle of cap in degrees
   amin
       minimum solid angle to probe in degrees.  holes larger than this area
       will not be missed with probability greater than pmax
   pmax
       holes of size amin will be missed with probability less than pmax

   returns
   -------
   maskflags
       2**0 is set if central point is inside the map
       2**1 is set if first quadrant is OK
       2**2 is set if second quadrant is OK
       2**3 is set if third quadrant is OK
       2**4 is set if fourth quadrant is OK
*/

// number of randoms do guarantee amin is missed with probability less than
// pmax
/*
static long get_check_quad_nrand(double area, double amin, double pmax)
{
    long nrand;
    double pmiss;

    pmiss = 1.0 - amin/area;
    if (pmiss > 1.e-10) {
        // how many points do we need in order for the
        // probability of missing the hole to be pmax?
        //      We need n such that (1-amin/a)^n = pmax
        double tmp = log10(pmax)/log10(pmiss);
        if (tmp < 20) tmp = 20;
        if (tmp > 20000) tmp = 20000;
        nrand = lround(tmp);
    } else {
        // we reach here often because the search area is very
        // close to or smaller than our minimum resolvable area
        // We don't want nrand to be less than say 20
        nrand = 20;
    }

    return nrand;
}
*/

// generate random points, return the fraction that were masked
static double get_quad_frac_masked(struct PyMangleMask* self,
                                   long nrand,
                                   const struct CapForRand *rcap,
                                   int quadrant)
{
    int status=1;
    long nmasked=0, i;
    double frac_masked=0;
    long double ra=0, dec=0, weight=0;
    int64 poly_id=0;
    struct Point pt={0};

    for (i=0; i<nrand; i++) {
        genrand_cap_radec(rcap, quadrant, &ra, &dec);
        point_set_from_radec(&pt, ra, dec);

        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        &poly_id, 
                                        &weight);

        if (poly_id < 0 || weight <= 0.) {
            nmasked += 1;
        }
    }

    frac_masked = ( (double)nmasked )/( (double)nrand );
    return frac_masked;
}

static PyObject*
PyMangleMask_check_quadrants(struct PyMangleMask* self, PyObject* args)
{
    int status=1;

    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* angle_degrees_obj=NULL;
    PyObject* maskflags_obj=NULL;
    npy_intp nra=0, ndec=0, nang=0, i=0, *maskflags_ptr=NULL;

    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double* ang_ptr=NULL;

    double density_degrees,
           max_masked_fraction, frac_masked;
    double area;
    struct Point pt;
    long double weight;
    int64 poly_id;
    struct CapForRand rcap;

    long nrand;
    int mask_flags=0;

    if (!PyArg_ParseTuple(args, (char*)"OOOdd",
                          &ra_obj,
                          &dec_obj,
                          &angle_degrees_obj,
                          &density_degrees, // number of randoms/square degree
                          &max_masked_fraction)) {
        return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!check_ra_dec_arrays(ra_obj,angle_degrees_obj,&ra_ptr,&nra,&ang_ptr,&nang)) {
        return NULL;
    }
    if (!(maskflags_obj=make_intp_array(nra, "maskflags", &maskflags_ptr))) {
        return NULL;
    }


    // area of a quadrant = 1/4 pi r^2
    for (i=0; i<nra; i++) {
        double dec_cen=dec_ptr[i];
        double ra_cen=ra_ptr[i];
        double ang=ang_ptr[i];
        mask_flags=0;

        area = 0.25*M_PI*ang*ang;
        nrand = (long) (area*density_degrees);


        // first check if the center itself is contained
        point_set_from_radec(&pt, ra_ptr[i], dec_ptr[i]);
        status=MANGLE_POLYID_AND_WEIGHT(self->mask, 
                                        &pt, 
                                        &poly_id, 
                                        &weight);

        if (poly_id >= 0) {
            int quadrant;
            mask_flags |= 1;

            CapForRand_from_radec(&rcap, ra_cen, dec_cen, ang);

            for (quadrant=1; quadrant <= 4; quadrant++) {
                frac_masked = get_quad_frac_masked(self, nrand, &rcap, quadrant);
                if (frac_masked < max_masked_fraction) {
                    mask_flags |= (1<<quadrant);
                }
            }
        }
        maskflags_ptr[i] = mask_flags;
    }

    return maskflags_obj;
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
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double weight=0;
    npy_intp poly_id=0;
    npy_intp ngood=0;
    long double theta=0, phi=0;


    if (!PyArg_ParseTuple(args, (char*)"L", &nrand)) {
        return NULL;
    }

    if (nrand <= 0) {
        PyErr_Format(PyExc_ValueError, 
                "nrand should be > 0, got (%ld)",(npy_intp)nrand);
        status=0;
        goto _genrand_cleanup;
    }

    if (!(ra_obj=make_longdouble_array(nrand, "ra", &ra_ptr))) {
        status=0;
        goto _genrand_cleanup;
    }
    if (!(dec_obj=make_longdouble_array(nrand, "dec", &dec_ptr))) {
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
    long double cthmin=0,cthmax=0,phimin=0,phimax=0;
    struct Point pt;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* tuple=NULL;
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    long double weight=0;
    npy_intp poly_id=0;
    npy_intp ngood=0;
    int num_contained=0;
    long double theta=0, phi=0;


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
    if (!radec_range_to_costhetaphi((long double)ramin,(long double)ramax,
				    (long double)decmin,(long double)decmax,
                                    &cthmin,&cthmax,&phimin,&phimax)) {
        status=0;
        goto _genrand_range_cleanup;
    }

    point_set_from_radec(&pt, ramin, decmin);
    status=MANGLE_POLYID_AND_WEIGHT(self->mask, &pt, &poly_id, &weight);
    if (poly_id >= 0) {
        num_contained += 1;
    }

    point_set_from_radec(&pt, ramin, decmax);
    status=MANGLE_POLYID_AND_WEIGHT(self->mask, &pt, &poly_id, &weight);
    if (poly_id >= 0) {
        num_contained += 1;
    }

    point_set_from_radec(&pt, ramax, decmin);
    status=MANGLE_POLYID_AND_WEIGHT(self->mask, &pt, &poly_id, &weight);
    if (poly_id >= 0) {
        num_contained += 1;
    }

    point_set_from_radec(&pt, ramax, decmax);
    status=MANGLE_POLYID_AND_WEIGHT(self->mask, &pt, &poly_id, &weight);
    if (poly_id >= 0) {
        num_contained += 1;
    }

    if (num_contained==0) {
        PyErr_Format(PyExc_ValueError,
                "no corners are contained within mask");
        status=0;
        goto _genrand_range_cleanup;
    }


    if (!(ra_obj=make_longdouble_array(nrand, "ra", &ra_ptr))) {
        status=0;
        goto _genrand_range_cleanup;
    }
    if (!(dec_obj=make_longdouble_array(nrand, "dec", &dec_ptr))) {
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


static PyObject *
PyMangleMask_pixels(struct PyMangleMask* self) {
    int status=1;
    PyObject *pixel_obj=NULL;
    npy_intp *pixel_ptr=NULL;
    struct Polygon *ply=NULL;
    npy_intp i;

    if (!(pixel_obj=make_intp_array(self->mask->poly_vec->size,"pixels", &pixel_ptr))) {
	status = 0;
	goto _pixels_cleanup;
    }

    ply = &self->mask->poly_vec->data[0];
    for (i=0;i<self->mask->poly_vec->size;i++) {
	pixel_ptr[i] = ply->pixel_id;
	ply++;
    }

 _pixels_cleanup:
    if (status != 1) {
	Py_XDECREF(pixel_obj);
	return NULL;
    }

    return PyArray_Return((PyArrayObject *)pixel_obj);
}

static PyObject *
PyMangleMask_weights(struct PyMangleMask* self) {
    int status=1;
    PyObject *weight_obj=NULL;
    long double *weight_ptr=NULL;
    struct Polygon *ply=NULL;
    npy_intp i;

    if (!(weight_obj=make_longdouble_array(self->mask->poly_vec->size,"weight", &weight_ptr))) {
	status = 0;
	goto _weights_cleanup;
    }

    ply = &self->mask->poly_vec->data[0];
    for (i=0;i<self->mask->poly_vec->size;i++) {
	weight_ptr[i] = ply->weight;
	ply++;
    }

 _weights_cleanup:
    if (status != 1) {
	Py_XDECREF(weight_obj);
	return NULL;
    }

    return PyArray_Return((PyArrayObject *)weight_obj);
}

static PyObject *
PyMangleMask_areas(struct PyMangleMask* self) {
    int status=1;
    PyObject *area_obj=NULL;
    long double *area_ptr=NULL;
    struct Polygon *ply=NULL;
    npy_intp i;

    if (!(area_obj=make_longdouble_array(self->mask->poly_vec->size,"area", &area_ptr))) {
	status = 0;
	goto _areas_cleanup;
    }

    ply = &self->mask->poly_vec->data[0];
    for (i=0;i<self->mask->poly_vec->size;i++) {
	area_ptr[i] = ply->area*R2D*R2D;
	ply++;
    }

 _areas_cleanup:
    if (status != 1) {
	Py_XDECREF(area_obj);
	return NULL;
    }

    return PyArray_Return((PyArrayObject *)area_obj);
}





static PyObject *
PyMangleMask_calc_simplepix(struct PyMangleMask *self, PyObject *args) {
    struct Point pt;
    PyObject *ra_obj=NULL;
    PyObject *dec_obj=NULL;
    PyObject *simplepix_obj=NULL;
    
    long double *ra_ptr=NULL;
    long double *dec_ptr=NULL;
    npy_intp *simplepix_ptr=NULL;
    npy_intp nra=0, ndec=0, i=0;

    if (!PyArg_ParseTuple(args, (char*)"OO", &ra_obj, &dec_obj)) {
        return NULL;
    }

    if (self->mask->pixeltype == 'u') {
	// not pixelized!
	PyErr_SetString(PyExc_ValueError,"Must be a pixelized file");
	return NULL;
    }

    if (!check_ra_dec_arrays(ra_obj,dec_obj,&ra_ptr,&nra,&dec_ptr,&ndec)) {
        return NULL;
    }
    if (!(simplepix_obj=make_intp_array(nra, "simplepix", &simplepix_ptr))) {
        return NULL;
    }

    for (i=0; i<nra; i++) {
	point_set_from_radec(&pt, *ra_ptr, *dec_ptr);

	*simplepix_ptr = get_pixel_simple(self->mask->pixelres,&pt);

	ra_ptr++;
	dec_ptr++;
	simplepix_ptr++;
    }

    return simplepix_obj;
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
        "    A numpy array of type 'f16'\n"
        "dec: array\n"
        "    A numpy array of type 'f16'\n"},
    {"polyid",            (PyCFunction)PyMangleMask_polyid,            METH_VARARGS, 
        "polyid(ra,dec)\n"
        "\n"
        "Check points against mask, returning the polygon id or -1.\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f16'\n"
        "dec: array\n"
        "    A numpy array of type 'f16'\n"},
    {"weight",            (PyCFunction)PyMangleMask_weight,            METH_VARARGS, 
        "weight(ra,dec)\n"
        "\n"
        "Check points against mask, returning the weight or 0.0\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f16'\n"
        "dec: array\n"
        "    A numpy array of type 'f16'\n"},
    {"contains",          (PyCFunction)PyMangleMask_contains,          METH_VARARGS, 
        "contains(ra,dec)\n"
        "\n"
        "Check points against mask, returning 1 if contained 0 if not\n"
        "\n"
        "parameters\n"
        "----------\n"
        "ra:  array\n"
        "    A numpy array of type 'f16'\n"
        "dec: array\n"
        "    A numpy array of type 'f16'\n"},

    {"check_quadrants",   (PyCFunction)PyMangleMask_check_quadrants,          METH_VARARGS, 
        "check_quadrants(ra,dec)\n"
        "\n"
        "check quadrants of a cap against the mask\n"},


    {"genrand",           (PyCFunction)PyMangleMask_genrand,           METH_VARARGS, 
        "genrand(nrand)\n"
        "\n"
        "Generate random points that are within the mask.\n"
        "\n"
        "parameters\n"
        "----------\n"
        "nrand: number\n"
        "    The number of random points to generate\n"
        "\n"
        "output\n"
        "------\n"
        "ra,dec arrays"},

    {"genrand_range",     (PyCFunction)PyMangleMask_genrand_range,     METH_VARARGS, 
        "genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "\n"
        "Generate random points inside the input range and the mask.\n"
        "The use case is when the mask area is small compared to the\n"
        "entire sphere; test points can be drawn from a region around \n"
        "the mask rather than the full sphere, improving speed.\n"
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
        "    The maximum dec\n"
        "\n"
        "output\n"
        "------\n"
        "ra,dec arrays"},

    {"read_weights", (PyCFunction)PyMangleMask_read_weights, METH_VARARGS,
     "read_weights(weightfile)\n"
     "\n"
     "Read weights from a separate weightfile.\n"},
    {"set_weights", (PyCFunction)PyMangleMask_set_weights, METH_VARARGS,
     "set_weights(weights)\n"
     "\n"
     "Set weights for all polygons.\n"},    
    {"get_filename",       (PyCFunction)PyMangleMask_filename,         METH_VARARGS, 
        "filename()\n"
        "\n"
        "Return the mask filename.\n"},

    {"get_weightfile",     (PyCFunction)PyMangleMask_weightfile, METH_VARARGS,
     "weightfile()\n"
     "\n"
     "Return the weight filename.\n"},


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
    {"get_pixels", (PyCFunction)PyMangleMask_pixels,  METH_VARARGS,
     "get_pixels()\n"
     "\n"
     "Return the array of pixels in the input file.\n"},
    {"get_weights", (PyCFunction)PyMangleMask_weights, METH_VARARGS,
     "get_weights()\n"
     "\n"
     "Return the array of weights in the input file.\n"},
    {"get_areas", (PyCFunction)PyMangleMask_areas, METH_VARARGS,
     "get_areas()\n"
     "\n"
     "Return the array of areas in the input file (in square degrees).\n"},
    {"calc_simplepix", (PyCFunction)PyMangleMask_calc_simplepix, METH_VARARGS,
     "calc_simplepix(ra,dec)\n"
     "\n"
     "Calculate simple pixel numbers, given pixelization scheme.\n"
     "\n"
     "parameters\n"
     "----------\n"
     "ra:  array\n"
     "    A numpy array of type 'f16'\n"
     "dec: array\n"
     "    A numpy array of type 'f16'\n"},
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
        "    import pymangle\n"
        "    m=pymangle.Mangle(mask_file, verbose=False)\n"
        "\n"
        "\n"
        "read-only properties\n"
        "--------------------\n"
        "    filename\n"
        "    area\n"
        "    npoly\n"
        "    is_pixelized\n"
        "    pixeltype\n"
        "    pixelres\n"
        "    maxpix\n"
        "    is_snapped\n"
        "    is_balkanized\n"
        "    weightfile\n"
        "\n"
        "See docs for each property for more details.\n"
        "\n"
        "methods\n"
        "----------------------\n"
        "    polyid(ra,dec)\n"
        "    weight(ra,dec)\n"
        "    polyid_and_weight(ra,dec)\n"
        "    contains(ra,dec)\n"
        "    genrand(nrand)\n"
        "    genrand_range(nrand,ramin,ramax,decmin,decmax)\n"
        "    calc_simplepix(ra,dec)\n"
        "    read_weights(weightfile)\n"
        "\n"
        "getters (correspond to properties above)\n"
        "----------------------------------------\n"
        "    get_filename()\n"
        "    get_weightfile()\n"
        "    get_area()\n"
        "    get_npoly()\n"
        "    get_is_pixelized()\n"
        "    get_pixeltype()\n"
        "    get_pixelres()\n"
        "    get_maxpix()\n"
        "    get_is_snapped()\n"
        "    get_is_balkanized()\n"
        "    get_pixels()\n"
        "    get_weights()\n"
        "    get_areas()\n"
        "\n"
        "setter (corresponding to property above)\n"
        "----------------------------------------\n"
        "    set_weights(weights)\n"
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






/*
 * Generate random points in the specified cap
 *
 * This is a straight function, not a method of a mangle mask
 *
 */

static PyObject*
PyMangle_genrand_cap(PyObject* self, PyObject* args)
{
    int status=1;
    PY_LONG_LONG nrand=0, i=0;
    double ra_cen=0,dec_cen=0,angle_degrees=0;
    int quadrant;
    PyObject* ra_obj=NULL;
    PyObject* dec_obj=NULL;
    PyObject* tuple=NULL;
    long double* ra_ptr=NULL;
    long double* dec_ptr=NULL;
    struct CapForRand rcap;

    if (!PyArg_ParseTuple(args, (char*)"Ldddi", 
                          &nrand, &ra_cen, &dec_cen, &angle_degrees, &quadrant)) {
        return NULL;
    }

    if (nrand <= 0) {
        PyErr_Format(PyExc_ValueError, 
                "nrand should be > 0, got (%ld)",(npy_intp)nrand);
        status=0;
        goto genrand_cap_cleanup;
    }
    if (angle_degrees <= 0 || angle_degrees > 180.) {
        PyErr_Format(PyExc_ValueError, 
                "angle_degrees should in [0,180), got (%ld)",(long)angle_degrees);
        status=0;
        goto genrand_cap_cleanup;
    }


    if (!(ra_obj=make_longdouble_array(nrand, "ra", &ra_ptr))) {
        status=0;
        goto genrand_cap_cleanup;
    }
    if (!(dec_obj=make_longdouble_array(nrand, "dec", &dec_ptr))) {
        status=0;
        goto genrand_cap_cleanup;
    }

    seed_random();

    CapForRand_from_radec(&rcap, ra_cen, dec_cen, angle_degrees);

    for (i=0; i<nrand; i++) {
        genrand_cap_radec(&rcap, quadrant, &ra_ptr[i], &dec_ptr[i]);
    }

genrand_cap_cleanup:
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



static PyMethodDef mangle_methods[] = {

    {"genrand_cap",       (PyCFunction)PyMangle_genrand_cap,         METH_VARARGS, 
        "genrand_cap(ra, dec, angle_degrees)\n"
        "\n"
        "get random points in the specified cap.\n"},

    {NULL}  /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_mangle",      /* m_name */
        "Defines Mangle related classes and some methods",  /* m_doc */
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


    PyMangleCapType.tp_new = PyType_GenericNew;
    PyMangleCapVecType.tp_new = PyType_GenericNew;
    PyManglePolygonType.tp_new = PyType_GenericNew;
    PyMangleMaskType.tp_new = PyType_GenericNew;

#if PY_MAJOR_VERSION >= 3
    if (PyType_Ready(&PyMangleCapType) < 0) {
        return NULL;
    }
    if (PyType_Ready(&PyMangleCapVecType) < 0) {
        return NULL;
    }
    if (PyType_Ready(&PyManglePolygonType) < 0) {
        return NULL;
    }
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
    if (PyType_Ready(&PyMangleCapType) < 0) {
        return;
    }
    if (PyType_Ready(&PyMangleCapVecType) < 0) {
        return;
    }
    if (PyType_Ready(&PyManglePolygonType) < 0) {
        return;
    }
    m = Py_InitModule3("_mangle", mangle_methods, 
            "This module defines a class to work with Mangle masks.\n"
            "and some generic functions.\n"
            "See docs for pymangle.Mangle for more details\n");
    if (m==NULL) {
        return;
    }
#endif

    Py_INCREF(&PyMangleMaskType);
    PyModule_AddObject(m, "Cap", (PyObject *)&PyMangleCapType);
    PyModule_AddObject(m, "CapVec", (PyObject *)&PyMangleCapVecType);
    PyModule_AddObject(m, "Polygon", (PyObject *)&PyManglePolygonType);
    PyModule_AddObject(m, "Mangle", (PyObject *)&PyMangleMaskType);

    import_array();
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
