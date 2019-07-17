#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
free_energy.py exports free energy expressions from a TDB thermodynamic databse
to the MOOSE input file format.

Usage:
  free_energy.py database.tdb [list of phases]

If no list of phases is supplied all phases are exported.
"""

import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath( __file__ )), '..', 'mms'))

from pycalphad import Database, Model
from fparser import *

# get commandline arguments
if len(sys.argv) < 2:
    print("usage:\n\t%s database.tdb [list of phases]\n" % sys.argv[0])
    print("Generate MOOSE input blocks for free energies from a supplied thermodynamic")
    print("database (.tdb) file. Further documentation can be found at")
    print("moose/modules/doc/content/python/CALPHAD_free_energies.md")
    sys.exit(1)

db_filename = sys.argv[1]
phases = sys.argv[2:]

# open the thermodynamic database file
try:
    db = Database(db_filename)
except IOError as e:
    print("Error opening database file.")
    print(e)
    sys.exit(1)

# compile list of phases to extract
available_phases = list(db.phases)
if len(phases) == 0:
    phases = available_phases
else:
    # check that the user specified phases provided in the input file
    if not set(phases) <= set(available_phases):
        print("The available phases are:")
        print(available_phases)
        sys.exit(1)

for phase in phases:
    print('  [./F_%s]' % phase)
    print('    type = DerivativeParsedMaterial')

    # get constituents
    constituents = list(set([i for c in db.phases[phase].constituents for i in c]))

    # create thermodynamic model
    m = Model(db, constituents, phase)

    # export fparser expression
    print("    function = '%s'" % fparser(m.ast))

    # print(variables
    print("    args = '%s'" % " ".join([v.name for v in m.variables]))
    print('  [../]')
