#!/usr/bin/env python3
from __future__ import print_function
from __future__ import division
import os
import argparse
import pandas
import matplotlib.pyplot as plt

def execute(name, ylabel, yscale=1, ext='pdf'):

    fig = plt.figure(figsize=[6,3], dpi=600, tight_layout=True)
    ax = fig.subplots()

    add_plot(ax, 'distribute_none', name, '(a) Baseline w/o sample data', yscale, marker='s')
    add_plot(ax, 'distribute_off', name, '(b) Non-distributed', yscale)
    add_plot(ax, 'distribute_on', name, '(c) Distributed', yscale)
    add_plot(ax, 'distribute_row', name, '(d) Iterative', yscale)

    ax.set_xlabel('Num. Processors', fontsize=14)
    ax.set_ylabel(ylabel, fontsize=14)
    ax.grid(True, color=[0.7]*3)
    ax.legend()

    fig.savefig('memory_{}.{}'.format(name, ext))

def add_plot(ax, prefix, name, label, yscale, marker='o'):
    """Show matplotlib plot of memory data"""
    dirname = os.path.abspath(os.path.dirname(__file__))
    data = pandas.read_csv(os.path.join(dirname, '{}.csv'.format(prefix)))
    ax.plot(data['n_procs'], data[name]/yscale, label=label, marker=marker)

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="Memory data for Sampler sample data methods.")
    parser.add_argument('-e', '--extension', default='pdf', type=str, help="The plot file extension.")
    args = parser.parse_args()

    execute('total', 'Memory (GiB)', 1024**3, args.extension)
    execute('per_proc', 'Memory (MiB)', 1024**2, args.extension)
