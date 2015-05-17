"""
classes
    Mangle:
        A class to work with mangle polygons

functions:
    genrand_cap:
        Generate random points in a spherical cap
"""
# we over-ride the init to deal with verbose
# we over-ride point checking codes force inputs to be arrays
#
# note we do *not* over-ride the genrand* functions,
# as they perform conversions as needed
#
# we also grab the doc strings from the C code as needed, 
# only writing new ones in the over-ridden methods

from __future__ import print_function

import numpy
from numpy import array
from . import _mangle

def genrand_cap(nrand, ra, dec, angle_degrees, quadrant=-1):
    """
    generate random points in a spherical cap

    parameters
    ----------
    nrand: scalar
        Number of random points to generate
    ra: scalar
        ra center of cap in degrees
    dec: scalar
        dec center of cap in degrees
    angle_degrees: scalar
        opening angle of cap in degrees 
    quadrant: scalar
        Quadrant in which to generate the points.  Set to
        1,2,3,4 to specify a quadrant, anything else to
        generate the full cap.  Default -1 (full cap)

    returns
    -------
    ra,dec: arrays
        the random points
    """
    return _mangle.genrand_cap(nrand, ra, dec, angle_degrees, quadrant)

class Mangle(_mangle.Mangle):
    __doc__=_mangle.Mangle.__doc__
    def __init__(self, filename, verbose=False):
        if verbose:
            verb=1
        else:
            verb=0
        super(Mangle,self).__init__(filename,verb)

    def read_weights(self, weightfile):
        """
        Read weights from a file with one weight on each line.

        parameters
        ----------
        weightfile: ascii file with one weight on each line.
                    Must have same number of weights as npoly.

        output
        ------
        none
        """
        
        super(Mangle,self).read_weights(weightfile)

    def polyid_and_weight(self, ra, dec):
        """
        Check points against mask, returning (poly_id,weight).

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.

        output
        ------
        polyd,weight tuple of arrays
        """
        ra = array(ra, ndmin=1, dtype='f16', copy=False, order='C')
        dec = array(dec, ndmin=1, dtype='f16', copy=False, order='C')
        return super(Mangle,self).polyid_and_weight(ra,dec)

    def polyid(self, ra, dec):
        """
        Check points against mask, returning the polygon id or -1.

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.

        output
        ------
        Array of poly ids
        """
        ra = array(ra, ndmin=1, dtype='f16', copy=False, order='C')
        dec = array(dec, ndmin=1, dtype='f16', copy=False, order='C')
        return super(Mangle,self).polyid(ra,dec)

    def weight(self, ra, dec):
        """
        Check points against mask, returning the weight or 0.

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.

        output
        ------
        Array of weights
        """
        ra = array(ra, ndmin=1, dtype='f16', copy=False, order='C')
        dec = array(dec, ndmin=1, dtype='f16', copy=False, order='C')
        return super(Mangle,self).weight(ra,dec)

    def contains(self, ra, dec):
        """
        Check points against mask, returning 1 if contained 0 if not

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.

        output
        ------
        Array of zeros or ones
        """
        # we specify order to force contiguous
        ra = array(ra, ndmin=1, dtype='f16', copy=False, order='C')
        dec = array(dec, ndmin=1, dtype='f16', copy=False, order='C')
        return super(Mangle,self).contains(ra,dec)

    def check_quadrants(self,
                        ra,
                        dec,
                        angle_degrees,
                        density=10.0*60.0**2,
                        max_masked_fraction=0.05):
        """
        Check points quadrants of the spherical cap against the mask
        using random points

        If more than a certain fraction of the random points is masked, the
        quadrant is considered masked.  The sensitivity can be adjusted
        by using a higher density of random points.

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.
        angle_degrees: scalar or array
            radius angle of cap in degrees.  Can be an array.
            should be same length as ra,dec
        density: scalar
            Density for random points placed in cap per square
            degree.  Default is 36000.0, which corresponds to
            10 per square arcminute.
        max_masked_fraction: scalar
            If more than this fraction of random points in the
            quadrant are masked, the quadrant is considered bad

        output
        ------
        maskflags: array
            2**0 is set if center of cap is inside the map
            2**1 is set if first quadrant is OK
            2**2 is set if second quadrant is OK
            2**3 is set if third quadrant is OK
            2**4 is set if fourth quadrant is OK
        """
        # we specify order to force contiguous
        ra = array(ra, ndmin=1, dtype='f16', copy=False, order='C')
        dec = array(dec, ndmin=1, dtype='f16', copy=False, order='C')
        angle_degrees = array(angle_degrees, ndmin=1, dtype='f16', copy=False, order='C')
        return super(Mangle,self).check_quadrants(ra,dec,angle_degrees,
                                                  density,max_masked_fraction)


    def calc_simplepix(self, ra, dec):
        """
        Calculate simple pixel numbers for list of ra, dec

        parameters
        ----------
        ra: scalar or array
            Right ascension in degrees.  Can be an array.
        dec: scalar or array
            Declination in degrees.  Can be an array.

        output
        ------
        Array of zeros or ones
        """
        ra = array(ra, ndmin=1, dtype='f16', copy=False)
        dec = array(dec, ndmin=1, dtype='f16', copy=False)
        return super(Mangle,self).calc_simplepix(ra,dec)

    def _set_weights(self,weights):
        # check length of array...
        npoly = _mangle.Mangle.get_npoly(self)
        if (weights.size != npoly) :
            raise IndexError("Must set weights for full list of %d polygons." % (npoly))

        # make long doubles
        weights = array(weights, ndmin=1, dtype='f16', copy=False)

        super(Mangle,self).set_weights(weights)


    filename = property(_mangle.Mangle.get_filename,doc="The mask filename")
    weightfile = property(_mangle.Mangle.get_weightfile,doc="The weight filename (optional)")
    area = property(_mangle.Mangle.get_area,doc="The area of the mask")
    npoly = property(_mangle.Mangle.get_npoly,doc="The number of polygons in the mask")
    is_pixelized = property(_mangle.Mangle.get_is_pixelized,doc="True if pixelized.")
    pixeltype = property(_mangle.Mangle.get_pixeltype,doc="The pixelization type, 'u' of unpixelized")
    pixelres = property(_mangle.Mangle.get_pixelres,doc="The pixel resolution, -1 if unpixelized")
    maxpix = property(_mangle.Mangle.get_pixelres,doc="The maximum pixel value")
    is_snapped = property(_mangle.Mangle.get_is_snapped,doc="True if snapped.")
    is_balkanized = property(_mangle.Mangle.get_is_balkanized,doc="True if balkanized.")
    areas = property(_mangle.Mangle.get_areas,doc="Area of pixels in mask.")
    weights = property(_mangle.Mangle.get_weights,_set_weights,doc="Weights of pixels in mask.")

class Cap(_mangle.Cap):
    """
    Class to represent a mangle Cap

    In order to support 128 bit, the input is an array [x,y,z,cm]
    """
    def __init__(self, data=None):
        """
        Initialize the cap with 128-bit data

        parameters
        ----------
        data: array or sequence
            An length 4 array of 128 bit floats, or convertable to that.
        """

        if data is not None:
            self.set(data)

    def set(self, data):
        """
        Initialize the cap with 128-bit data

        parameters
        ----------
        data: array or sequence
            An length 4 array of 128 bit floats, or convertable to that.
        """
        data = array(data, ndmin=1, dtype='f16', copy=False)
        if data.size != 4:
            raise ValueError("capdata must be an array of length 4, got %d" % data.size)

        super(Cap,self).set(data)

class CapVec(_mangle.CapVec):
    """
    methods

    cv=CapVec(3)

    len(cv)
    cv[index] = data_or_cap
    cap = cv[index]

    # getters and setters same as above
    cv.set(index,data_or_cap)
    cap=cv.get(index)
    """
    def set(self, index, data):
        """
        Set the cap at the specified element

        parameters
        ----------
        data: sequence or Cap
            Data to set the cap in the vector.  Can be a sequence/array of [x,y,z,cm]
            or a Cap
        """

        sz=self.size()
        if index > (sz-1):
            raise IndexError("index %s out of bounds: [0,%s)" % (index,sz))

        if isinstance(data, Cap):
            cap=data
        else:
            cap=Cap(data)

        self._set_cap(index, data)

    def get(self, index):
        """
        get a copy of the Cap specified by the index

        parameters
        ----------
        data: sequence or Cap
            Data to set the cap in the vector.  Can be a sequence/array of [x,y,z,cm]
            or a Cap
        """

        sz=self.size()
        if index > (sz-1):
            raise IndexError("index %s out of bounds: [0,%s)" % (index,sz))

        return self._get_cap(index)

    def __setitem__(self, index, data):
        """
        Set the cap at the specified element

        parameters
        ----------
        data: sequence or Cap
            Data to set the cap in the vector.  Can be a sequence/array of [x,y,z,cm]
        """
        self.set(index, data)

    def __getitem__(self, index):
        """
        get a copy of the Cap specified by the index
        """
        return self.get(index)

    def __len__(self):
        """
        length of CapVec
        """
        return self.size()

class Polygon(_mangle.Polygon):
    """
    class representing a mangle Polygon

    methods

    poly=Polygon(poly_id, pixel_id, weight, cap_vec)

    len(poly)
    # copy of a cap
    cap = poly[index]

    # getters and setters same as above
    cap=cv.get(index)
    """
    def __init__(self, poly_id, pixel_id, weight, cap_vec):
        if not isinstance(cap_vec, CapVec):
            raise ValueError("cap_vec must be of "
                             "type CapVec, got %s" % type(cap_vec))

        wtarr = array(weight, ndmin=1, dtype='f16', copy=False)

        super(Polygon,self).__init__(poly_id,
                                     pixel_id,
                                     wtarr,
                                     cap_vec)

    def get(self, index):
        """
        get a copy of the Cap specified by the index

        parameters
        ----------
        data: sequence or Cap
            Data to set the cap in the vector.  Can be a sequence/array of [x,y,z,cm]
            or a Cap
        """

        return self.get(index)

    def __getitem__(self, index):
        """
        get a copy of the Cap specified by the index
        """
        sz=self.size()
        if index > (sz-1):
            raise IndexError("index %s out of bounds: [0,%s)" % (index,sz))

        return self._get_cap(index)

    def __len__(self):
        """
        length of CapVec
        """
        return self.size()


