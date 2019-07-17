#!/usr/bin/env python3
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
    for n in xrange(n_start, n_stop+1):
        n_samples = 2**n
        exe_args = ['-i', infile, 'Samplers/mc/n_samples={}'.format(n_samples),
                    'MultiApps/runner/mode={}'.format(mode),
                    'Outputs/file_base={}'.format(mode)]

        print('{} {}'.format(exe, ' '.join(exe_args)))
        t = time.time()
        out = mooseutils.run_executable(exe, exe_args, mpi=mpi, suppress_output=True)
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

    # These are the runs that were performed for the documentation, they were run using "cone" and
    # it took several hours for the jobs to complete.
    execute('full_solve.i', 'full_solve_memory_serial', 0, 10, ['normal', 'batch-reset', 'batch-restore'])
    execute('full_solve.i', 'full_solve_memory_mpi', 5, 14, ['normal', 'batch-reset', 'batch-restore'], 32)

    execute('transient.i', 'transient_memory_serial', 0, 10, ['normal', 'batch-restore'])
    execute('transient.i', 'transient_memory_mpi', 5, 14, ['normal', 'batch-restore'], 32)
