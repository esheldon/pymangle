__version__ = 'v0.9.2'

from . import mangle
from .mangle import Mangle, genrand_cap
from .version import __version__
from . import test

__doc__ = mangle.__doc__
