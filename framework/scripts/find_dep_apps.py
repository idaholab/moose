#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys

if __name__ == '__main__':
    MOOSE_DIR = os.environ.get('MOOSE_DIR', os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..')))
    sys.path.append(os.path.join(MOOSE_DIR, 'python'))
    from TestHarness import *
    if len(sys.argv) == 2:
        dep_apps = findDepApps(sys.argv[1], False)
        print(dep_apps)
