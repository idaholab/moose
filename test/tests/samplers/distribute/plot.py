#!/usr/bin/env python
from __future__ import print_function
from __future__ import division
import os
import pandas
import matplotlib.pyplot as plt

def execute(name, ylabel, yscale=1):

    fig = plt.figure(figsize=[6,3], dpi=600, tight_layout=True)
    ax = fig.subplots()

    add_plot(ax, 'distribute_none', name, 'Baseline w/o sample data', yscale)
    add_plot(ax, 'distribute_off', name, 'Non-distributed', yscale)
    add_plot(ax, 'distribute_on', name, 'Distributed', yscale)

    ax.set_xlabel('Num. Processors', fontsize=14)
    ax.set_ylabel(ylabel, fontsize=14)
    ax.grid(True, color=[0.7]*3)
    ax.legend()

    fig.savefig('memory_{}.svg'.format(name))

def add_plot(ax, prefix, name, label, yscale):
    """Show matplotlib plot of memory data"""
    dirname = os.path.abspath(os.path.dirname(__file__))
    data = pandas.read_csv(os.path.join(dirname, '{}.csv'.format(prefix)))
    ax.plot(data['n_procs'], data[name]/yscale, label=label, marker='o')

if __name__ == '__main__':
    execute('total', 'Memory (GiB)', 1024**3)
    execute('per_proc', 'Memory (MiB)', 1024**2)
