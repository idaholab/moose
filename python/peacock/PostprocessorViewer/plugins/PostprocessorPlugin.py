#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import peacock
import mooseutils
from PyQt5 import QtWidgets

class PostprocessorPlugin(peacock.base.Plugin):
    """
    The base class for creating a plugin for the PostprocessorViewer.
    """

    def __init__(self):
        super(PostprocessorPlugin, self).__init__()

        # Initialize member variables
        self._figure = None
        self._axes = None
        self._axes2 = None

        # The default layout name
        self.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Maximum)
        self.setFixedWidth(520)
        self.setMainLayoutName('LeftLayout')

    def onFigureCreated(self, figure, axes, axes2):
        """
        Stores the created figure and axes for use by the plugin. This slot becomes connected to the
        figureCreated signal that is emitted by the FigurePlugin.

        Args:
            figure[plt.Figure]: The matplotlib figure object that postprocessor data is to be displayed.
            axes[plt.Axes]: The Axes with y-data displayged on left side of figure.
            axes2[plt.Axes]: The Axes with y-data displayed on the right side of figure.
        """
        self._figure = figure
        self._axes = axes
        self._axes2 = axes2

    def repr(self):
        """
        Return imports and script data
        """
        return [], []

    def figure(self):
        """
        Returns the current matplotlib Figure object.
        """
        return self._figure

    def axes(self, index=None):
        """
        Return the axes object(s).
        """
        if index in [0, 'x', 'y']:
            return self._axes
        elif index in [1, 'y2']:
            return self._axes2
        return self._axes, self._axes2

    def axis(self, name):
        """
        Return the pyplot.Axis object.

        Args:
            name[str]: 'x', 'y', or 'y2'.
        """
        if name == 'x':
            return self.axes(0).get_xaxis()
        elif name == 'y':
            return self.axes(0).get_yaxis()
        elif name == 'y2':
            return self.axes(1).get_yaxis()
        mooseutils.mooseError("Unknown axis name, must use: 'x', 'y', or 'y2'.")
