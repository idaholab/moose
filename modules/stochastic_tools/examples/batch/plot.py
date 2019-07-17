#!/usr/bin/env python3
from __future__ import print_function
import argparse
import pandas
import matplotlib.pyplot as plt
import multiprocessing
import mooseutils

GOLD = True # When True the files from the gold directory are used, which should contain the
            # data that is used in the documentation

def plotter(prefix, outfile, modes, names, labels, ylabel):
    """Show matplotlib plot of memory data"""

    fig = plt.figure(figsize=[9,4], dpi=600)
    ax = fig.subplots()

    for name, label in zip(names, labels):
        for i, mode in enumerate(modes):
            fstr = 'gold/{}_{}.csv' if GOLD else '{}_{}.csv'
            data = pandas.read_csv(fstr.format(prefix, mode))
            ax.plot(data['n_samples'], data[name], label='{} ({})'.format(mode, label), markersize=8,
                    marker='osd'[i])

    ax.set_xlabel('Num. Samples', fontsize=12)
    ax.set_ylabel(ylabel, fontsize=12)
    ax.set_xscale('log', basex=2)
    ax.set_yscale('log', basey=2)
    ax.grid(True, color=[0.7]*3)
    ax.legend()

    fig.savefig(outfile)

if __name__ == '__main__':

    # Full Solve
    plotter('full_solve_memory_serial', '../../doc/content/media/full_solve_time_serial.svg',
            ['normal', 'batch-restore', 'batch-reset'], ['time'], ['Time'], 'Time (sec.)')

    plotter('full_solve_memory_mpi', '../../doc/content/media/full_solve_memory_mpi.svg',
            ['normal', 'batch-restore', 'batch-reset'], ['total', 'max_proc'], ['Total', 'Max'], 'Memory (MiB)')

    plotter('full_solve_memory_serial', '../../doc/content/media/full_solve_memory_serial.svg',
            ['normal', 'batch-restore', 'batch-reset'], ['total'], ['Total'], 'Memory (MiB)')

    plotter('full_solve_memory_mpi', '../../doc/content/media/full_solve_time_mpi.svg',
            ['normal', 'batch-restore', 'batch-reset'], ['time'], ['Time'], 'Time (sec.)')

    # Transient
    plotter('transient_memory_serial', '../../doc/content/media/transient_serial_time.svg',
            ['normal', 'batch-restore'], ['time'], ['Time'], 'Time (sec.)')

    plotter('transient_memory_serial', '../../doc/content/media/transient_memory_serial.svg',
            ['normal', 'batch-restore'], ['total'], ['Total'], 'Memory (MiB)')

    plotter('transient_memory_mpi', '../../doc/content/media/transient_mpi_time.svg',
            ['normal', 'batch-restore'], ['time'], ['Time'], 'Time (sec.)')

    plotter('transient_memory_mpi', '../../doc/content/media/transient_memory_mpi.svg',
            ['normal', 'batch-restore'], ['total', 'max_proc'], ['Total', 'Max'], 'Memory (MiB)')
