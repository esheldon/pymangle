pymangle
========

Simple python code to read and work with Mangle masks.

The underlying code is in C for speed, and is based
on some C++ code written by Martin White.

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
