#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
    print(prefix, time.time() - start_time, 'sec.')

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
