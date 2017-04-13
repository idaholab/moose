import matplotlib.pyplot as plt
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
    plt.grid(True)
    plt.grid(True, which='minor', color='b')

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
