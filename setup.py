import distutils
from distutils.core import setup, Extension, Command
import os
import numpy


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

description="Simple python code to read and work with Mangle masks."

rstname='README.rst'
if os.path.exists(rstname):
    desc_file=rstname
else:
    desc_file='README.md'

with open(desc_file) as fobj:
    long_description=fobj.read()



setup(name="pymangle", 
      packages=['pymangle'],
      description=description,
      long_description=long_description,
      version=__version__,
      license = "GPL",
      ext_modules=[ext],
      include_dirs=numpy.get_include())


