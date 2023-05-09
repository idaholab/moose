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

        self.label_to_slope = {}
        self.label_to_intercept = {}

    def plot(self, df, label=None, title=None, num_fitted_points=None, slope_precision=3, **kwargs):
        num_y_columns = len(df.columns) - 1

        if label:
            if num_y_columns > 1:
                if not isinstance(label, list):
                    raise TypeError("For multiple y-data label must be a list")


            if isinstance(label, list) and num_y_columns != len(label):
                raise IOError("The length of the label and the number of y columns must be the same")

            if not isinstance(label, list):
                label = [label]

        x = np.array(df[df.columns[0]])
        lines = []

        for i in range(1,len(df.columns)):
            y = np.array(df[df.columns[i]])

            if label is None:
                this_label = 'line-{}'.format(len(lines))
            else:
                this_label = label[i-1]

            if num_fitted_points is not None:
                coeffs = self._fit(x[-num_fitted_points:], y[-num_fitted_points:])
            else:
                coeffs = self._fit(x, y)

            slope = coeffs[0]
            intercept = coeffs[1]
            self.label_to_slope.update({this_label:slope})
            self.label_to_intercept.update({this_label:intercept})

            this_label = '{}: {:.{precision}f}'.format(this_label, slope, precision=slope_precision)

            lines.append(self._axes.plot(x, y, label=this_label, **kwargs)[0])

        if title:
            self._axes.set_title(title)

        self._axes.legend()
        return lines

    def set_title(self, title):
        self._axes.set_title(title)

    def _fit(self, x, y):
        """
        Apply the fit and report the slope.

        Key, value Options:
          x[float]: The x-position in data coordinates.
          y[float]: The y-position in data coordinates.
        """

        # Perform fit
        coefficients = np.polyfit(np.log10(x), np.log10(y), 1)
        return coefficients

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
