#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://www.mooseframework.org
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from os import getenv
from os.path import exists, join
import json
from glob import glob
try:
    from conda_build.metadata import MetaData
    from conda.cli.python_api import Commands, run_command
except ImportError:
    print(0)
    sys.exit(0)
try:
    MOOSE_DIR = getenv("MOOSE_DIR", sys.argv[1])
    CONDA_PREFIX = getenv('CONDA_PREFIX')
    exists(CONDA_PREFIX)
except TypeError:
    print(0)
    sys.exit(0)


def parse_recipe_meta_data(meta_file_path):
    """Construct package version from metadata."""
    meta = MetaData(meta_file_path)
    pkg_name = meta.meta['package']['name']
    pkg_version = meta.meta['package']['version']
    pkg_build = meta.meta['build'].get('string', '*')
    full_name = '-'.join([pkg_name, pkg_version, pkg_build]) + '.json'
    return full_name, pkg_name, pkg_version, pkg_build


def get_recipe_versions():
    """Iterate through all conda/**/meta.yaml files and return package info tuple."""
    conda_folder = join(MOOSE_DIR, 'conda')
    recipes = glob(join(conda_folder, '**/meta.yaml'), recursive=True)
    for recipe in recipes:
        yield parse_recipe_meta_data(recipe)


def latest_remote_libmesh_version(output):
    """Extract version number from latest conda libmesh."""
    results = json.loads(output)
    latest_libmesh = results['moose-libmesh'][-1]
    return latest_libmesh['version']


def check_remote_conda(pkg_name):
    """Search conda remote for all moose-libmesh package information."""
    search_tuple = run_command(Commands.SEARCH, ['-f', pkg_name, '--json'])
    if search_tuple[2] > 0:
        return None
    return latest_remote_libmesh_version(search_tuple[0])


def inform_user_to_update():
    """User should update conda."""
    msg = "moose-libmesh may be outdated. run conda update -n moose --all"
    print(msg, file=sys.stderr)


def inform_user_to_wait():
    """User will have to wait."""
    msg = """
There appears to be a libMesh update in progress for
the branch of MOOSE you are operating on (only the
master branch contains publicly available moose-libmesh
packages).

You must provide your own libMesh either by:

    1. Uninstall moose-libmesh and build it manually via
       moose/scripts/update_and_rebuild_libmesh.sh
or
    2. Use `conda build` to build your own moose-libmesh
       package.
"""
    print(msg, file=sys.stderr)


def main():
    """Check to see if user is using conda and environment is current."""
    for full_name, pkg_name, pkg_version, pkg_build in get_recipe_versions():
        current_conda = join(CONDA_PREFIX, 'conda-meta', full_name)

        # Just handle libmesh for now. Then if needed, add
        # functionality to handle packages with hashes in its' version.
        if pkg_name != 'moose-libmesh':
            continue

        if not exists(current_conda):
            remote_version = check_remote_conda(pkg_name)
            if remote_version is None or remote_version == pkg_version:
                inform_user_to_update()
            else:
                inform_user_to_wait()
                return 1
    return 0


if __name__ == '__main__':
    # This script should act as an informational note only, allowing the user to
    # continue as frequently as possible (early versions of Conda do not work)
    try:
        print(main())
    except:
        print(0)
        sys.exit(0)
