import glob
import os

from setuptools import setup
from setuptools.extension import Extension
from setuptools.command import build_ext


ext = Extension("pymangle._mangle", ["pymangle/_mangle.c",
                                     "pymangle/mangle.c",
                                     "pymangle/cap.c",
                                     "pymangle/polygon.c",
                                     "pymangle/pixel.c",
                                     "pymangle/point.c",
                                     "pymangle/stack.c",
                                     "pymangle/rand.c"])


class BuildExt(build_ext.build_ext):
    '''Custom build_ext command to hide the numpy import
    Inspired by http://stackoverflow.com/a/21621689/1860757'''
    def finalize_options(self):
        '''add numpy includes to the include dirs'''
        build_ext.build_ext.finalize_options(self)
        import numpy as np
        self.include_dirs.append(np.get_include())
        self.include_dirs.extend(glob.glob("pymangle/*h"))


exec(open('pymangle/version.py').read())

description = "Simple python code to read and work with Mangle masks."

dname = os.path.dirname(__file__)
rstname = os.path.join(dname, 'README.rst')
if os.path.exists(rstname):
    desc_file = rstname
else:
    desc_file = os.path.join(dname, 'README.md')

with open(desc_file) as fobj:
    long_description = fobj.read()


setup(name="pymangle",
      packages=['pymangle'],
      description=description,
      long_description=long_description,
      version=__version__,
      license="GPL",
      install_requires=['numpy', ],
      ext_modules=[ext],
      setup_requires=['numpy', ],
      cmdclass={'build_ext': BuildExt},
      include_package_data=True,
      # include_dirs=numpy.get_include()
      )
