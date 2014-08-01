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

import numpy
from numpy import array
from . import _mangle
__doc__=_mangle.__doc__

def genrand_cap(nrand, ra, dec, angle_degrees, quadrant=0):
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
        generate the full cap.  Default 0 (full cap)

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
