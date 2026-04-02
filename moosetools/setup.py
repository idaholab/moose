# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Python setuptools setup for moosetools."""

from setuptools import find_packages, setup

from .setup_capabilities_core import CAPABILITIES_CORE_EXTENSION

if __name__ == "__main__":
    setup(
        name="moosetools",
        version="0.1.1",
        description="TODO",
        package_dir={"": "src"},
        packages=find_packages(where="src"),
        ext_modules=[CAPABILITIES_CORE_EXTENSION],
        include_package_data=True,
        package_data={
            "moosetools.capabilities": ["*.pyi", "py.typed"],
        },
        zip_safe=False,
    )
