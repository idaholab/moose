#!/usr/bin/env python

"""
free_energy.py exports free energy expressions from a TDB thermodynamic databse
to the MOOSE input file format.

Usage:
  free_energy.py database.tdb [list of phases]

If no list of phases is supplied all phases are exported.
"""

import sys
from pycalphad import Database, Model
from utils.fparser import fparser

# get commandline arguments
try:
    db_filename = sys.argv[1]
except IndexError:
    print "usage:\n\tfree_energy.py database.tdb [list of phases]"
    sys.exit(1)

phases = sys.argv[2:]

# open the thermodynamic database file
try:
    db = Database(db_filename)
except IOError, e:
    print "Error opening database file."
    print e
    sys.exit(1)

# compile list of phases to extract
available_phases = list(db.phases)
if len(phases) == 0:
    phases = available_phases
else:
    # check that the user specified phases provided in the input file
    if not set(phases) <= set(available_phases):
        print "The available phases are:"
        print available_phases
        sys.exit(1)

for phase in phases:
    print '  [./F_%s]' % phase
    print '    type = DerivativeParsedMaterial'
    print '    block = 0'

    # get constituents
    constituents = list(set([i for c in db.phases[phase].constituents for i in c]))

    # create thermodynamic model
    m = Model(db, constituents, phase)

    # export fparser expression
    print "    function = '%s'" % fparser(m.ast)[2]

    # print variables
    print "    args = '%s'" % " ".join([v.name for v in m.variables])
    print '  [../]'
