
import os
import sys
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

sources = ['parse.cc', 'lex.cc', 'hit.pyx']
cxxargs = ['-Wno-everything', '-std=c++11']

# the -mmacosx-version-min flag is to get around the fact that distutils tries to compile c code
# with the same flags that the python interpreter being used was compiled with.  For our conda
# install, it was built with support for a minimum mac os x version of 10.6 - which makes it try
# to use gcc's libstdc++ (which is old and doesn't support c++11) instead of clang's libc++ (which
# properly supports c++11).  See
# https://stackoverflow.com/questions/33738885/python-setuptools-not-including-c-standard-library-headers.
if 'clang' in os.environ['CXX']:
    cxxargs.append('-mmacosx-version-min=10.9')

setup(
    ext_modules = cythonize([Extension('hit', sources, language='c++', libraries=['stdc++'], extra_compile_args=cxxargs)])
)
