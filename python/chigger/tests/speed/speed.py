#!/usr/bin/env python
#pylint: disable=missing-docstring
#################################################################
#                   DO NOT MODIFY THIS HEADER                   #
#  MOOSE - Multiphysics Object Oriented Simulation Environment  #
#                                                               #
#            (c) 2010 Battelle Energy Alliance, LLC             #
#                      ALL RIGHTS RESERVED                      #
#                                                               #
#           Prepared by Battelle Energy Alliance, LLC           #
#             Under Contract No. DE-AC07-05ID14517              #
#              With the U. S. Department of Energy              #
#                                                               #
#              See COPYRIGHT for full restrictions              #
#################################################################
import os
import time
import chigger

filename = os.path.join(os.getenv('MOOSE_DIR'), 'modules', 'phase_field', 'examples', 'grain_growth', 'grain_growth_2D_graintracker_out.e')
variables = ['bnds']
blocks = ['0']
N = 10
#filename = os.path.join(os.getenv('MOOSE_DIR'), '..', 'relap-7', 'pkl_1loop_out_displaced.e')
#variables = ['alpha_liquid']
#block = ['Pipe20']
#N =1
reader = chigger.exodus.ExodusReader(filename, timestep=0)

#reader.update()
#print reader.getVariableInformation()
#print reader.getBlockInformation()

def read_all_times(prefix=''):
    start_time = time.time()
    for i in range(N):
        reader.update()
        times = reader.getTimes()
        for t in times:
            reader.update(timestep=None, time=t)
    print prefix, time.time() - start_time, 'sec.'

# Default settings
read_all_times('Default:')

reader.setOptions(squeeze=True)
read_all_times('squeeze=False:')

reader.setOptions(variables=variables)
reader.setOptions(squeeze=False)
read_all_times('Variable:')

#reader.setOptions(variables=None, block=block)
reader.setOptions(variables=None)
read_all_times('Block:')

#reader.setOptions(variables=variables, block=block)
reader.setOptions(variables=variables)
read_all_times('Variable and Block:')
