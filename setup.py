import setuptools
import numpy

sources = [
    "pymangle/_mangle.c",
    "pymangle/mangle.c",
    "pymangle/cap.c",
    "pymangle/polygon.c",
    "pymangle/pixel.c",
    "pymangle/point.c",
    "pymangle/stack.c",
    "pymangle/rand.c",
]

ext = setuptools.Extension(
    "pymangle._mangle",
    sources,
    include_dirs=[numpy.get_include()],
)

exec(open('pymangle/version.py').read())

description = "A python code to read and work with Mangle masks."

with open('README.md') as fobj:
    long_description = fobj.read()

setuptools.setup(
    name="pymangle",
    packages=['pymangle'],
    description=description,
    long_description=long_description,
    long_description_content_type='text/markdown',
    version=__version__,
    license="GPL",
    ext_modules=[ext],
    include_dirs=numpy.get_include(),
)
