#!/usr/bin/env python3
from __future__ import print_function
import time
import argparse
import pandas
import matplotlib.pyplot as plt
import multiprocessing
import mooseutils


def execute(infile, outfile, n_samples, processors, test_type):
    """Helper for running a memory study with increasing MPI ranks."""

    data = dict(n_procs=[], n_samples=[], total=[], per_proc=[], max_proc=[], time=[])
    exe = mooseutils.find_moose_executable_recursive()

    for n_procs in processors:
        file_base = '{}_{}'.format(infile[:-2], n_procs)
        exe_args = ['-i', infile,
                    'Outputs/file_base={}'.format(file_base),
                    'Postprocessors/test/test_type={}'.format(test_type),
                    'Samplers/sampler/num_rows={}'.format(int(n_samples))]

        print('mpiexec -n {} {} {}'.format(n_procs, exe, ' '.join(exe_args)))
        t = time.time()
        out = mooseutils.run_executable(exe, *exe_args, mpi=n_procs, suppress_output=True)
        t = time.time() - t

        local = pandas.read_csv('{}.csv'.format(file_base))
        data['n_procs'].append(n_procs)
        data['n_samples'].append(n_samples)
        data['total'].append(local['total'].iloc[-1])
        data['per_proc'].append(local['per_proc'].iloc[-1])
        data['max_proc'].append(local['max_proc'].iloc[-1])
        data['time'].append(t)

    df = pandas.DataFrame(data, columns=['n_procs', 'n_samples', 'total', 'per_proc', 'max_proc', 'time'])
    df.to_csv('{}.csv'.format(outfile), index=False)

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="Memory data for Sampler sample data methods.")
    parser.add_argument('-r', '--rows', default=1e5, type=int, help="The number of rows (default: 1e5).")
    parser.add_argument('-p', '--processors', default=[1,4], type=int, nargs='+', help="List of number of processors to use (default: [1,4]).")
    args = parser.parse_args()

    execute('distribute.i', 'distribute_none', 1, args.processors, 'getGlobalSamples')
    execute('distribute.i', 'distribute_off', args.rows, args.processors, 'getGlobalSamples')
    execute('distribute.i', 'distribute_on', args.rows, args.processors, 'getLocalSamples')
    execute('distribute.i', 'distribute_row', args.rows, args.processors, 'getNextLocalRow')
