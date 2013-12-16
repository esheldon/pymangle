pymangle
========

Simple python code to read and work with Mangle masks.

The underlying code is in C for speed, and is based on some C++ code written by
Martin White.

Currently supports ascii mangle polygon files, with and without pixelization.

examples
--------

    import pymangle

    # read a mangle polygon file
    m=pymangle.Mangle("mask.ply")

    # test an ra,dec point against the mask
    good = m.contains(200.0, 0.0)

    # test arrays of ra,dec points against the mask
    ra=numpy.array([200.0, 152.7])
    dec=numpy.array([0.0, -15.0])
    good = m.contains(ra, dec)

    # get the polygon ids
    ids = m.polyid(ra,dec)
    
    # get the weights
    weights = m.weight(ra,dec)

    # get poth polyids and weights
    ids, weights = m.polyid_and_weight(ra, dec)

    # generate random points    
    ra_rand, dec_rand = m.genrand(1000)

    # generate randoms from the mask and with a
    # rectangle.  Speeds things up if your mask is relatively
    # small compared to the full sphere; choose
    # the box just big enough to contain the mask.

    ra_min=200.0
    ra_max=210.0
    dec_min=0.0
    dec_max=10.0
    ra_rand, dec_rand = m.genrand_range(1000,ra_min,ra_max,dec_min,dec_max)

build and install python library
--------------------------------

    python setup.py install --prefix=/some/path

build and install the stand alone routines
------------------------------------------

    # just build
    python build.py

    # also install.  Note different order from above
    python build.py --prefix=/some/path install

    # clean up
    python build.py clean

