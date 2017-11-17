#!/usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np

filenames = ['biharmonic_centered_dirichlet.csv',
             'biharmonic_centered_weak_bc.csv']

for filename in filenames:
    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    data = np.genfromtxt(filename, delimiter=',')

    # Extract vectors
    logh = np.log10(data[:,0])
    log_L2 = np.log10(data[:,1])
    log_H1 = np.log10(data[:,2])

    # Compute linear fits
    l2_fit = np.polyfit(logh, log_L2, 1)
    h1_fit = np.polyfit(logh, log_H1, 1)

    # Make log-log plots
    ax1.plot(logh, log_L2, linewidth=2, marker='o', label=r'$L^2$ error')
    ax1.plot(logh, np.poly1d(l2_fit)(logh), linestyle='--', color='black')
    ax1.plot(logh, log_H1, linewidth=2, marker='o', label=r'$H^1$ error')
    ax1.plot(logh, np.poly1d(h1_fit)(logh), linestyle='--', color='black')

    # Put text labels on plot
    ax1.text(-1.6, -6, '{:.2f}'.format(l2_fit[0]))
    ax1.text(-1.6, -3.75, '{:.2f}'.format(h1_fit[0]))

    # Add axis labels, legend, and print
    ax1.set_xlabel('log(h)')
    ax1.legend(loc='upper left')
    plt.savefig(filename.rsplit( ".", 1)[0] + '.pdf')
