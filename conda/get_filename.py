#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse, os, sys
from conda_build.metadata import MetaData

def verifyArgs(args):
    if not args.recipe:
        print('Supply a path to a meta.yaml recipe file\n')
        sys.exit(1)
    return args

def parseArguments(args=None):
    parser = argparse.ArgumentParser(description='Conda Build Name Getter',
                                     epilog='Parses supplied conda meta.yaml recipe to return package filename.')
    parser.add_argument('-r', '--recipe', help='Path to conda meta.yaml file')
    return verifyArgs(parser.parse_args(args))

def main():
    args = parseArguments()
    if os.path.exists(args.recipe):
        meta = MetaData(args.recipe)
        print('-'.join([meta.meta['package']['name'],
                        meta.meta['package']['version'],
                        meta.meta['build'].get('string', '*') ]) + '.tar.bz2')
    return 0

if __name__ == '__main__':
  sys.exit(main())
