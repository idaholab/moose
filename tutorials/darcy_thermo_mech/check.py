#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import filecmp
import subprocess
import argparse

def compare(dir0, dir1, link=False):
    """
    Recursively compare the files in a directory to check for identical files.

    Inputs:
        dir0: The base directory, identical files in dir1 will be linked to this directory if the
              'create' flag is True.
        dir1: The directory to compare, links will be created here if the 'create' flag is True.
        linkR: When True a symlink is created.
    """
    for root, _, files in os.walk(dir1):
        for fname in files:
            full_name1 = os.path.join(root, fname)
            full_name0 = full_name1.replace(dir1, dir0)
            if (os.path.isfile(full_name0) and
                os.path.isfile(full_name1) and
                not os.path.islink(full_name1) and
                filecmp.cmp(full_name0, full_name1)):

                # Define the link source, use split to handle filenames without extensions
                source = os.path.relpath(full_name0, root)
                link_name = os.path.basename(full_name1)
                cmd = ['ln', '-s', source, link_name]
                if link:
                    os.remove(full_name1)
                    subprocess.call(cmd, cwd=root) # os.symlink doesn't handle files w/o extension
                    print ' '.join(cmd)
                else:
                    print 'REQUIRED: {}'.format(' '.join(cmd))

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="A helper tool for keeping the symlinks correct.")
    parser.add_argument('--link', action='store_true', help="Generate missing symlinks.")
    options = parser.parse_args()

    # Compare the sorted directories and show/create symlinks
    steps = sorted([d for d in os.listdir(os.path.dirname(__file__)) if os.path.isdir(d)])
    for i in range(1, len(steps)):
        compare(steps[i-1], steps[i], link=options.link)
