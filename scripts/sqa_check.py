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
import sys
import subprocess
import argparse

moose_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(moose_dir, 'python'))
import mooseutils

def get_options():
    """Command-line options."""
    parser = argparse.ArgumentParser(description='SQA Requirement checking tool.')

    parser.add_argument('-d', '--directory', type=str, default=mooseutils.git_root_dir(),
                        help="The directory to search.")
    parser.add_argument('-r', '--remote', type=str, default='origin',
                        help="The name of the git remote to compare against.")
    parser.add_argument('-b', '--branch', type=str, default='devel',
                        help="The name of the branch to compare against.")
    parser.add_argument('--specs', type=list, default=['tests'],
                        help="The name of the specification files to consider.")
    parser.add_argument('--skip', nargs='+', default=[],
                        help="Partial directory paths to ignore.")
    parser.add_argument('--duplicates', action='store_true',
                        help='Enable duplicate requirement check')
    return parser.parse_args()

if __name__ == '__main__':

    opt = get_options()

    cmd = ['git', 'fetch', opt.remote]
    subprocess.call(cmd)

    count = mooseutils.sqa_check(opt.directory, opt.remote, opt.branch, opt.specs, opt.skip)

    if opt.duplicates:
        count += mooseutils.sqa_check_requirement_duplicates(opt.directory, opt.specs, opt.skip)

    sys.exit(count)
