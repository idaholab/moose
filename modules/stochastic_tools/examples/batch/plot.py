#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import argparse
import pandas
import matplotlib.pyplot as plt
import multiprocessing
import mooseutils

# Use this for updating documentation plots, after running execute.py
LOCATION = os.path.join('..', '..', '..', '..', 'large_media', 'stochastic_tools')
EXT = 'svg'
GOLD = True

# Use this for re-creating plots for paper
#LOCATION = '.'
#EXT = 'pdf'
#GOLD = True

def plotter(prefix, outdir, suffix, ext, modes, names, labels, ylabel):
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

    outfile = os.path.join(outdir, '{}{}.{}'.format(prefix, '_' + suffix if suffix else '' , ext))
    fig.savefig(outfile)

if __name__ == '__main__':

    # Full Solve
    plotter('full_solve_memory_serial', LOCATION, 'time', EXT,
            ['normal', 'batch-restore', 'batch-reset'], ['time'], ['Time'], 'Time (sec.)')

    plotter('full_solve_memory_serial', LOCATION, '', EXT,
            ['normal', 'batch-restore', 'batch-reset'], ['total'], ['Total'], 'Memory (MiB)')

    plotter('full_solve_memory_mpi', LOCATION, 'time', EXT,
            ['normal', 'batch-restore', 'batch-reset'], ['time'], ['Time'], 'Time (sec.)')

    plotter('full_solve_memory_mpi', LOCATION, '', EXT,
            ['normal', 'batch-restore', 'batch-reset'], ['total', 'max_proc'], ['Total', 'Max'], 'Memory (MiB)')

    # Transient
    plotter('transient_memory_serial', LOCATION, 'time', EXT,
            ['normal', 'batch-restore'], ['time'], ['Time'], 'Time (sec.)')

    plotter('transient_memory_serial', LOCATION, '', EXT,
            ['normal', 'batch-restore'], ['total'], ['Total'], 'Memory (MiB)')

    plotter('transient_memory_mpi', LOCATION, 'time', EXT,
            ['normal', 'batch-restore'], ['time'], ['Time'], 'Time (sec.)')

    plotter('transient_memory_mpi', LOCATION, '', EXT,
            ['normal', 'batch-restore'], ['total', 'max_proc'], ['Total', 'Max'], 'Memory (MiB)')
