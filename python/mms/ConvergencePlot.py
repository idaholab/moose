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
import collections
import pandas
import numpy as np

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
    Line = collections.namedtuple('Line', 'x y label')

    def __init__(self, xlabel='x', ylabel='y', fontsize=12, fit=True):

        self._figure = plt.figure(figsize=(10,6), facecolor='w')
        self._axes = plt.gca()

        self._axes.set_yscale('log')
        self._axes.set_xscale('log')

        # Add axis labels
        plt.xlabel(xlabel, fontsize=fontsize)
        plt.ylabel(ylabel, fontsize=fontsize)

        # Adjust tick mark fonts
        for tick in self._axes.xaxis.get_major_ticks() + self._axes.yaxis.get_major_ticks():
            tick.label.set_fontsize(fontsize)

        # Apply grid marks
        plt.grid(True, which='both', color=[0.8]*3)

    def plot(self, x, y, label=None, invert_x=True, **kwargs):


        if invert_x:
            x = 1./np.array(x)
        else:
            x = np.array(x)

        y = np.array(y)

        if label is None:
            label = 'line-{}'.format(len(self._axes.lines))


        slope = self._fit(x, y)
        label = '{}: {:.3f}'.format(label, slope)

        #self._data.append(ConvergencePlot.Line(x=x, y=y, label=label))

        line, = self._axes.plot(x, y, label=label, **kwargs)

        self._axes.legend()
        return line

    def _fit(self, x, y):
        """
        Apply the fit and report the slope.

        Key, value Options:
          x[float]: The x-position in data coordinates.
          y[float]: The y-poisition in data coordinates.
        """

        # Perform fit
        coefficients = np.polyfit(np.log10(x), np.log10(y), 1)
        return coefficients[0]

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
