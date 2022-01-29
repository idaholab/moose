#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os

# Locate MOOSE directory, the default is the location of MOOSE as determined by this file, the realpath follows symlinks
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..', '..')))
if not os.path.exists(MOOSE_DIR):
    print('Failed to locate MOOSE, specify the MOOSE_DIR environment variable.')
    sys.exit(1)
os.environ['MOOSE_DIR'] = MOOSE_DIR

# Append MOOSE python directory
MOOSE_PYTHON_DIR = os.path.join(MOOSE_DIR, 'python')
if MOOSE_PYTHON_DIR not in sys.path:
    sys.path.append(MOOSE_PYTHON_DIR)

# Append custom extensions
THM_PYTHON_DIR = os.path.join(MOOSE_DIR, 'modules', 'thermal_hydraulics', 'python', 'MooseDocs', 'extensions')
if THM_PYTHON_DIR not in sys.path:
    sys.path.append(THM_PYTHON_DIR)

# Always operate from the directory of them moosedocs.py script, this allows the script to be
# run from any location. CIVET runs 'modules/doc/moosedocs.py check' from the root directory.
os.chdir(os.path.abspath(os.path.dirname(__file__)))

from MooseDocs import main
if __name__ == '__main__':
    sys.exit(main.run())
