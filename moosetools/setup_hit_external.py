# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Python setuptools setup for moosetools.hit._core."""

import os
import platform

try:
    from setuptools import Extension
except ImportError:
    raise SystemExit(
        "Cannot setup moosetools.hit without setuptools; ",
        "install setuptools in your python environment",
    )


def hit_extension() -> Extension:
    """Build the Extension for moosetools.hit.hit."""
    # WASP, either from the environment or in-tree
    wasp_dir = os.environ.get(
        "WASP_DIR",
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../wasp/install")),
    )
    if not os.path.isdir(wasp_dir):
        raise SystemExit(f"WASP is not available in {wasp_dir}")
    wasp_lib_dir = os.path.join(wasp_dir, "lib")
    wasp_include_dir = os.path.join(wasp_dir, "include")

    # Find each wasp library
    lib_suffix = "dylib" if platform.system() == "Darwin" else "so"
    wasp_libs = [
        file.replace("lib", "", 1).replace(f".{lib_suffix}", "", 1)
        for file in os.listdir(os.path.join(wasp_dir, "lib"))
        if (file.endswith(f".{lib_suffix}"))
    ]

    dir = "src/moosetools/hit"
    external_src_dir = os.path.join(dir, "external/src")
    external_include_dir = os.path.join(dir, "external/include")
    return Extension(
        "moosetools.hit.hit",
        sources=[
            *[os.path.join(external_src_dir, v) for v in os.listdir(external_src_dir)],
        ],
        include_dirs=[external_include_dir, wasp_include_dir],
        library_dirs=[wasp_lib_dir],
        runtime_library_dirs=[wasp_lib_dir],
        libraries=wasp_libs,
        extra_compile_args=["-std=c++17"],
    )


HIT_EXTERNAL_MODULES = [hit_extension()]
"""The external modules for the moosetools.hit module."""

if __name__ == "__main__":
    from setuptools import setup

    setup(ext_modules=HIT_EXTERNAL_MODULES)
