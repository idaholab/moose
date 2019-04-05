#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib.pyplot as plt
import glob
import pandas
import numpy as np


def plot(filename, x='h', y='error', xlabel=None, ylabel=None, output=None, show=True, **kwargs):
    """
    Helper function for creating convergence plot from filename(s).

    Inputs:
        filename[str]: The CSV file(s) to open, when a * is in the name glob is used to open all
                       the files and extract the last value from the columns, this is useful when
                       evaluating transient solves
        x,y [str]: The column names to extract from the PP CSV file(s)
        xlabel, ylabel [str]: The name of the x and y axis labels
        output[str]: The filename to output
        show[bool]: Flag for showing the plot
        fit[bool]: Flag for showing line fit
        fontsize[int]: The plot font size (default: 16)
    """

    # Handle multiple files for transient solutions
    if '*' in filename:
        filenames = sorted(glob.glob('level_set_mms*.csv'))
        n = len(filenames)
        error = np.zeros(n)
        length = np.zeros(n)
        for i, filename in enumerate(filenames):
            csv = pandas.read_csv(filename)
            length[i] = csv[x].iloc[-1]
            error[i] = csv[y].iloc[-1]

    else:
        csv = pandas.read_csv(filename)
        length = csv[x]
        error = csv[y]

    if xlabel is None:
      xlabel = x
    if ylabel is None:
      ylabel = y

    fig = ConvergencePlot(length, error, xlabel=xlabel, ylabel=ylabel, **kwargs)

    if output is not None:
      fig.save(output)

    if show:
      fig.show()

class ConvergencePlot(object):
    """
    A tool for making convergence plots.

    Args:
        x[np.array]: The x data of the graph (e.g., dofs)
        y[np.array]: The y data of the graph (e.g., L2_error)

    Key, value Options:
        xlabel[str]: The label for the x-axis
      ylabel[str]: The label for the y-axis
    """
    def __init__(self, x, y, xlabel='x', ylabel='y', fontsize=16, fit=True):

        self._x = np.array(x)
        self._y = np.array(y)

        self._figure = plt.figure(figsize=(10,6), facecolor='w')
        self._line = plt.plot(self._x, self._y, '-ob', markersize=8)[0]
        self._axes = plt.gca()

        self._axes.set_yscale('log')
        self._axes.set_xscale('log')

        # Add axis labels
        plt.xlabel(xlabel, fontsize=fontsize)
        plt.ylabel(ylabel, fontsize=fontsize)

        # Adjust tick mark fonts
        for tick in self._axes.xaxis.get_major_ticks() + self._axes.yaxis.get_major_ticks():
            tick.label.set_fontsize(18)

        # Apply grid marks
        plt.grid(True, which='both', color=[0.8]*3)

        if fit:
            self.fit()

    def fit(self, **kwargs):
        """
        Apply the fit and report the slope.

        Key, value Options:
          x[float]: The x-position in data coordinates.
          y[float]: The y-poisition in data coordinates.
        """

        # Perform fit
        coefficients = np.polyfit(np.log10(self._x), np.log10(self._y), 1)

        # Display slope
        x = kwargs.get('x', min(self._x))
        y = kwargs.get('y', max(self._y))
        self._axes.text(x, y, 'slope = {}'.format(coefficients[0]), fontsize=15)

    def save(self, filename):
        """
        Save figure to a file.

        Args:
          filename[str]: The destination file.
        """
        plt.savefig(filename)

    def show(self):
        """
        Display the plot.
        """
        plt.show()
