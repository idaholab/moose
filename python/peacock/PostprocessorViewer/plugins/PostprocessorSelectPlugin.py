import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import itertools

from PyQt5 import QtCore, QtWidgets
from PostprocessorPlugin import PostprocessorPlugin
from LineGroupWidget import LineGroupWidget

import mooseutils

class PostprocessorSelectPlugin(QtWidgets.QWidget, PostprocessorPlugin):
    """
    Widget that contains the toggles for plotting the individual postprocessor data.

    This builds a scrollable box containing LineGroupWidget objects, these toggles control the visibility
    and style of the postprocessor line plots.
    """

    #: pyqtSignal: Emitted when plot is refreshed, contains the x/y/y2 axis variable names
    variablesChanged = QtCore.pyqtSignal(list, list, list)

    #: pyqtSignal: Emitted when the LineGroupWidgets change the plot.
    axesModified = QtCore.pyqtSignal()

    def __init__(self):
        super(PostprocessorSelectPlugin, self).__init__()

        # Setup this widget
        policy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.MinimumExpanding)
        policy.setVerticalStretch(100) # We want this widget to be as big as possible vertically
        self.setSizePolicy(policy)

        # An iteratable color cycle for setting the default line style and color
        self.color_cycle = None

        # Member variables
        self._groups = []  # list of ListGroupWidget objects

        # The box and layout that will contain the line toggles
        self.LineGroups = QtWidgets.QFrame()
        self.LineGroupsLayout = QtWidgets.QVBoxLayout()
        self.LineGroupsLayout.setSpacing(10);
        self.LineGroupsLayout.setContentsMargins(10, 10, 10, 10);
        self.LineGroups.setLayout(self.LineGroupsLayout)

        # Creates the area that will be scrollable
        self.Scroll = QtWidgets.QScrollArea()
        self.Scroll.setWidget(self.LineGroups)

        # Main layout to contain the scroll area
        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 10, 0, 10)
        self.MainLayout.addWidget(self.Scroll)
        self.setLayout(self.MainLayout)

        # Call the setup methods
        self.setup()

    def initialize(self, data):
        """
        Called when new data is being supplied to the widget.

        Args:
            data[list]: A list of PostprocessorDataWidget files.
        """
        # Enable the widget
        super(PostprocessorSelectPlugin, self).initialize(data)

        # Remove existing widgets
        for group in self._groups:
            self.LineGroupsLayout.removeWidget(group)
            group.setParent(None)
        self._groups = []

        self.color_cycle = itertools.product(['-', '--', '-.', ':'], plt.cm.Paired(np.linspace(0, 1, 11)))

        # Create the group widgets for each available variable
        for d in data:
            group = LineGroupWidget(self.axes(), d, self.color_cycle)
            self.LineGroupsLayout.addWidget(group)
            self._groups.append(group)
            group.initialized.connect(self.updateGeometry)
            group.variablesChanged.connect(self.updateVariables)
            group.axesModified.connect(self.axesModified)

        self.updateGeometry()

    def onTimeChanged(self, time):
        """
        Update the time in the GroupLineWidgets.
        """
        for group in self._groups:
            group.plot(time=time)

    def onCurrentChanged(self, index):
        """
        Enables/disables the update timer base on the active state of the tab.
        """
        active = self._index == index
        for group in self._groups:
            group._data.setTimerActive(active)

    @QtCore.pyqtSlot()
    def updateVariables(self):
        """
        Updates the complete list of active variables for x/y axis labels.
        """

        n = len(self._groups)
        x_vars = [[]]*n
        y_vars = [[]]*n
        y2_vars = [[]]*n
        for i in range(n):
            group = self._groups[i]
            if group.isValid():
                x, y, y2 = group.getAxisLabels()
                x_vars[i] = [x]
                y_vars[i] = y
                y2_vars[i] = y2
        self.variablesChanged.emit(x_vars, y_vars, y2_vars)

    @QtCore.pyqtSlot()
    def minimumSizeHint(self):
        """
        Update the widget width and the widths of the LineGroups to keep everything lined up.
        """

        # Compute the max width
        # We let the vertical stretch factor take care of the height
        size = QtWidgets.QWidget.minimumSizeHint(self)
        margins = self.LineGroupsLayout.getContentsMargins()
        w = size.width()
        for group in self._groups:
            w = max(w, group.width())

        # Set all groups to the same width
        for group in self._groups:
            group.setFixedWidth(w)

        # Return the minimum size for this widget
        size.setWidth(w + margins[0] + margins[2])
        return size

    def repr(self):
        """
        Produce the script items for this widget.
        """

        output = []
        imports = []

        for group in self._groups:
            out, imp = group.repr()
            output += out
            imports += imp

        return output, imports

    def _setupScroll(self, qobject):
        """
        Setup method for the scroll area widget.
        """
        qobject.setWidgetResizable(True)
        qobject.setFrameShape(QtWidgets.QFrame.NoFrame)
        qobject.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)

    def _setupLineToggleGroupBox(self, qobject):
        """
        Setup method for the group box storing the line toggle widgets.
        """
        qobject.setAutoFillBackground(True)


def main(filenames, reader=mooseutils.VectorPostprocessorReader):
    """
    Create widgets for running PostprocessorSelectPlugin
    """
    """
    Run FigurePlugin by itself.
    """
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    from FigurePlugin import FigurePlugin

    widget = PostprocessorViewer(reader, timeout=None, plugins=[FigurePlugin, PostprocessorSelectPlugin])
    widget.initialize(filenames)
    control = widget.currentWidget().PostprocessorSelectPlugin
    window = widget.currentWidget().FigurePlugin
    window.setFixedSize(QtCore.QSize(625, 625))
    widget.show()

    return control, widget, window


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    filenames = ['../../../tests/input/vpp_*.csv']
    _, widget, _ = main(filenames)
    app.exec_()
    os.remove('tmp_001.csv')
