#!/usr/bin/python
import os, sys, argparse

if(os.path.islink(sys.argv[0])):
  pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
else:
  pathname = os.path.dirname(sys.argv[0])
  pathname = os.path.abspath(pathname)

MOOSE_DIR = os.path.abspath(os.path.join(pathname, '../../../'))
sys.path.append(os.path.join(MOOSE_DIR, 'python'))

# Requires ~/projects/moose/python to be added to PYTHONPATH
from PresentationBuilder import base

if __name__ == '__main__':

  # List of input files to compile into a single presentation
  files = ['input/cover.i',
           'input/overview.i',
           'input/problem.i',
           'input/fem.i',
           'input/anatomy.i',
           'input/kernels.i',
           'input/coords.i',
           'input/step01.i',
           'input/peacock.i',
           'input/input_parameters.i',
           'input/step02.i',
           'input/materials.i',
           'input/step03.i',
           'input/aux_system.i',
           'input/coupling.i',
           'input/step04.i',
           'input/adapt.i',
           'input/modules.i']

  # Create the presentation containing the entire moose workshop
  merger = base.PresentationMerger('darcy_thermo_mech.i', files, style='inl', title='ThermomechanicalDarcyFlow')
  merger.write()
