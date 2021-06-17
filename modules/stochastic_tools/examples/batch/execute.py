#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function
import time
import argparse
import pandas
import matplotlib.pyplot as plt
import multiprocessing
import mooseutils

def runner(infile, outfile, n_start, n_stop, mode, mpi):

    data = dict(n_samples=[], total=[], per_proc=[], max_proc=[], time=[])

    exe = mooseutils.find_moose_executable_recursive()
    for n in range(n_start, n_stop+1):
        n_samples = 2**n
        exe_args = ['-i', infile, 'Samplers/mc/num_rows={}'.format(n_samples),
                    'MultiApps/runner/mode={}'.format(mode),
                    'Outputs/file_base={}'.format(mode)]

        print('{} {}'.format(exe, ' '.join(exe_args)))
        t = time.time()
        out = mooseutils.run_executable(exe, *exe_args, mpi=mpi, suppress_output=True)
        t = time.time() - t

        local = pandas.read_csv('{}.csv'.format(mode))
        data['n_samples'].append(n_samples)
        data['total'].append(local['total'].iloc[-1])
        data['per_proc'].append(local['per_proc'].iloc[-1])
        data['max_proc'].append(local['max_proc'].iloc[-1])
        data['time'].append(t)

    df = pandas.DataFrame(data, columns=['n_samples', 'total', 'per_proc', 'max_proc', 'time'])
    df.to_csv('{}_{}.csv'.format(outfile, mode), index=False)

def execute(infile, outfile, n_start, n_stop, modes, mpi=None):
    """Run input for memory data"""

    if mpi:
        for mode in modes:
            runner(infile, outfile, n_start, n_stop, mode, mpi)

    else:
        jobs = []
        for mode in modes:
            p = multiprocessing.Process(target=runner, args=(infile, outfile, n_start, n_stop, mode, mpi))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

if __name__ == '__main__':

    # This took about 8 hours on:
    #   Mac Pro (2019)
    #   2.5 GHz 28-Core Intel Xeon W
    #   240 GB 2933 MHz DDR4
    execute('full_solve.i', 'full_solve_memory_serial', 0, 10, ['normal', 'batch-reset', 'batch-restore'])
    execute('full_solve.i', 'full_solve_memory_mpi', 5, 16, ['normal', 'batch-reset', 'batch-restore'], 28)

    execute('transient.i', 'transient_memory_serial', 0, 10, ['normal', 'batch-restore'])
    execute('transient.i', 'transient_memory_mpi', 5, 16, ['normal', 'batch-restore'], 28)
