import distutils
from distutils.core import setup, Extension, Command
import os
import numpy

data_files=[]

ext=Extension("pymangle._mangle", ["pymangle/_mangle.c",
                                   "pymangle/mangle.c",
                                   "pymangle/cap.c",
                                   "pymangle/polygon.c",
                                   "pymangle/pixel.c",
                                   "pymangle/point.c",
                                   "pymangle/stack.c",
                                   "pymangle/rand.c"],
              include_dirs=[numpy.get_include()])
              

exec(open('pymangle/version.py').read())

setup(name="pymangle", 
      packages=['pymangle'],
      version=__version__,
      data_files=data_files,
      ext_modules=[ext],
      include_dirs=numpy.get_include())


