#!/usr/bin/python
import os, sys, argparse

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
           'input/peacock.i',
           'input/input_parameters.i',
           'input/materials.i',
           'input/aux_system.i',
           'input/coupling.i',
           'input/adapt.i',
           'input/modules.i']

  # Create the presentation containing the entire moose workshop
  merger = base.PresentationMerger('darcy_thermo_mech.i', files, style='inl', title='ThermomechanicalDarcyFlow')
  merger.write()
