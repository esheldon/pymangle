pymangle
========

Simple python code to read and work with Mangle masks.

The underlying code is in C for speed, and is based on some C++ code written by
Martin White.  Martin has posted his C++ code at
https://github.com/martinjameswhite/litemangle

Currently supports ascii mangle polygon files, with and without pixelization.
The code works in python2 and python3

Authors: Erin Sheldon.  Long double support added by Eli Rykoff.

examples
--------

```python
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

# generate randoms from the mask and with the additional constraint that they
# are within the specified rectangle.  This speeds things up if your mask is
# relatively small compared to the full sphere; choose the box just big enough
# to contain the mask.

ra_min=200.0
ra_max=210.0
dec_min=0.0
dec_max=10.0
ra_rand, dec_rand = m.genrand_range(1000,ra_min,ra_max,dec_min,dec_max)

# get the polygon weights
weights = m.weights

# read in a new set of weights
m.read_weightfile(weightfile)

# set the weights

m.weights = weight_array
```

build and install python library
--------------------------------

```
python setup.py install --prefix=/some/path
```
