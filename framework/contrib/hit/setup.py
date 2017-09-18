
from distutils.core import setup
from Cython.Build import cythonize
from Cython.Distutils import build_ext
from distutils.extension import Extension

import os
os.system('gcc --version')

setup(
    ext_modules = cythonize([Extension('hit', ['hit.pyx'])])
)

