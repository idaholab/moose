try:
    from setuptools import setup
    from setuptools import Extension
except ImportError:
    from distutils.core import setup
    from distutils.extension import Extension

setup(
    ext_modules=[
        Extension(
            "hit",
            ["hit.cpp", "lex.cc", "parse.cc"],
            extra_compile_args=['-std=c++11'],
        )
    ]
)
