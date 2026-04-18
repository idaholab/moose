# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement a helper for setting up/building a module's external dependencies."""

import os
import subprocess
import sys
import sysconfig


def setup_external(script_path: str, module_name: str):
    """Build the external dependencies a moosetools package in tree."""
    moosetools_dir = os.path.abspath(
        os.path.join(os.path.abspath(os.path.dirname(__file__)), "..", "..", "..")
    )
    lib_suffix = sysconfig.get_config_var("EXT_SUFFIX")
    lib_path = os.path.join(moosetools_dir, f"src/moosetools/{module_name}{lib_suffix}")

    print(f"Building and linking {lib_path}...")
    cmd = [
        sys.executable,
        script_path,
        "build",
        "--force",
        "--build-lib",
        "src",
    ]
    process = subprocess.run(
        cmd,
        cwd=moosetools_dir,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )

    if process.returncode != 0:
        print(f"Build command '{' '.join(cmd)}' failed:")
        print(process.stdout)
        raise SystemExit(1)

    assert os.path.isfile(lib_path), f"Library {lib_path} does not exist"
