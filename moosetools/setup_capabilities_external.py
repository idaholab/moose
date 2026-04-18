# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Python setup for external modules of moosetools.capabilities."""

import os

try:
    from setuptools import Extension
except ImportError:
    raise SystemExit(
        "Cannot setup moosetools.capabilities without setuptools; ",
        "install setuptools in your python environment",
    )


def external_extension() -> Extension:
    """Build the Extension for moosetools.capabilities._external."""
    dir = "src/moosetools/capabilities"
    external_src_dir = os.path.join(dir, "external/src")
    external_include_dir = os.path.join(dir, "external/include")
    return Extension(
        "moosetools.capabilities._external",
        sources=[
            os.path.join(dir, "_external.cpp"),
            *[os.path.join(external_src_dir, v) for v in os.listdir(external_src_dir)],
        ],
        include_dirs=[external_include_dir],
        extra_compile_args=[
            "-std=c++17",
            "-DFOR_MOOSETOOLS",
            "-DMOOSESTRINGUTILS_NO_LIBMESH",
        ],
    )


CAPABILITIES_EXTERNAL_MODULES = [external_extension()]
"""The external modules for the moosetools.hit module."""

if __name__ == "__main__":
    from setuptools import setup

    setup(ext_modules=CAPABILITIES_EXTERNAL_MODULES)
