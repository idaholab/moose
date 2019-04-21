#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg
from PyQt5 import QtCore, QtWidgets

from .PostprocessorPlugin import PostprocessorPlugin

class FigurePlugin(QtWidgets.QWidget, PostprocessorPlugin):
    """
    This plugin contains the matplotlib figure and axes.

    This plugin should be created first because it contains the figure and axes that are sent to the
    other plugins via the figureCreated signal.
    """

    #: pyqtSignal: Emitted when the Axes objects are created.
    figureCreated = QtCore.pyqtSignal(plt.Figure, plt.Axes, plt.Axes)

    def __init__(self):
        super(FigurePlugin, self).__init__()

        # Create the figure
        self._figure = plt.Figure(facecolor='white')
        self._axes = self._figure.add_subplot(111)
        self._axes2 = self._axes.twinx()
        self.setMainLayoutName('RightLayout') # used by plugin system to place widget

        # The 'main' layout
        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.MainLayout.setSpacing(5)
        self.MainLayout.setAlignment(QtCore.Qt.AlignRight)
        self.setLayout(self.MainLayout)
        self.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)

        self.FigureCanvas = FigureCanvasQTAgg(self._figure)
        self.FigureCanvas.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)

        self.MainLayout.addWidget(self.FigureCanvas)
        self.setFixedWidth(QtWidgets.QWIDGETSIZE_MAX) # reset the fixed width so it can be resized

        self.setup()

    def onSetData(self, data):
        """
        Initialize the FigurePlugin by drawing the axes and emitting the 'figureCreated' signal.

        This signal must be called here, because all of the other objects must be created prior to this
        signal being emitted. In the future this may allow us to have multiple figures or switch plot
        types from 2D to 3D.
        """
        self.figureCreated.emit(self._figure, self._axes, self._axes2)
        self.onAxesModified()

    def onTimeChanged(self, time):
        """
        Re-draw the figure if the time was changed.
        """
        self.draw()

    def onAxesModified(self):
        """
        Re-draw the figure if the axes were modified.
        """
        self.draw()

    def draw(self):
        # Update Axes limits and y-axis visibility
        for ax in self.axes():
            ax.relim()
            ax.autoscale(enable=None)
            ax.get_yaxis().set_visible(len(ax.lines) > 0)

        # Set x-axis visibility
        if (not self._axes.get_yaxis().get_visible()) and (not self._axes2.get_yaxis().get_visible()):
            self._axes.get_xaxis().set_visible(False)
        else:
            self._axes.get_xaxis().set_visible(True)

        self._figure.canvas.draw()

    def clear(self):
        """
        Clear all plots from the Axes.
        """
        self._axes.clear()
        self._axes2.clear()

    def repr(self):
        """
        Return the matplotlib script for reproducing the figure and axes.
        """

        imports = ['import matplotlib.pyplot as plt']

        output = []
        output += ["# Create Figure and Axes"]
        output += ["figure = plt.figure(facecolor='white')"]
        output += ['axes0 = figure.add_subplot(111)']

        if self._axes2.has_data():
            output += ['axes1 = axes0.twinx()']

        return output, imports

    def onWrite(self, filename):
        """
        Saves the figure to the supplied filename.

        Args:
            filenmae[str]: A pdf/png/py file to export.
        """
        if filename.endswith('.py'):
            self.parent().write(filename)
        else:
            self._figure.savefig(filename)

def main():
    """
    Run FigurePlugin by itself.
    """
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    import mooseutils

    widget = PostprocessorViewer(mooseutils.VectorPostprocessorReader, plugins=[FigurePlugin])
    widget.onSetFilenames([])
    widget.currentWidget().FigurePlugin.setFixedSize(QtCore.QSize(375, 375))
    widget.show()
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main()
    widget.currentWidget().Figure.axes(0).plot([1,2], [3,4], '--b')
    widget.currentWidget().Figure.onAxesModified()
    sys.exit(app.exec_())
