#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from PyQt5 import QtCore, QtWidgets
import mooseutils
from .PostprocessorPlugin import PostprocessorPlugin
from .AxisSettingsWidget import AxisSettingsWidget

class AxisTabsPlugin(QtWidgets.QTabWidget, PostprocessorPlugin):

    #pyqtSignal: Emited when axes has been modified
    axesModified = QtCore.pyqtSignal()

    def __init__(self):
        super(AxisTabsPlugin, self).__init__()

        self.XAxisTab = AxisSettingsWidget('x', 0)
        self.YAxisTab = AxisSettingsWidget('y', 0)
        self.Y2AxisTab = AxisSettingsWidget('y', 1)

        self.addTab(self.XAxisTab, 'X Axis')
        self.addTab(self.YAxisTab, 'Y Axis (left)')
        self.addTab(self.Y2AxisTab, 'Y Axis (right)')

        self.setup()

        self.XAxisTab.axesModified.connect(self.axesModified)
        self.YAxisTab.axesModified.connect(self.axesModified)
        self.Y2AxisTab.axesModified.connect(self.axesModified)

    def onFigureCreated(self, figure, axes, axes2):
        """
        Sets the Axes object on the widgets within each tab
        """
        PostprocessorPlugin.onFigureCreated(self, figure, axes, axes2)
        self.XAxisTab.onFigureCreated(axes)
        self.YAxisTab.onFigureCreated(axes)
        self.Y2AxisTab.onFigureCreated(axes2)

    def onVariablesChanged(self, x_vars, y_vars, y2_vars):
        """
        Update the default variable names.
        """

        # Create a single list of uniqne names
        x = []; y = []; y2 = []
        for i in range(len(x_vars)):
            mooseutils.unique_list(x, x_vars[i])
            mooseutils.unique_list(y, y_vars[i])
            mooseutils.unique_list(y2, y2_vars[i])

        # Update the default labels
        self.XAxisTab.setLabelDefault(x)
        self.YAxisTab.setLabelDefault(y)
        self.Y2AxisTab.setLabelDefault(y2)
        self.XAxisTab._callbackLabel()
        self.YAxisTab._callbackLabel()
        self.Y2AxisTab._callbackLabel()

        # Update the limits
        if self._axes != None:
            self.blockSignals(True)
            self.XAxisTab.setLimit(0)
            self.XAxisTab.setLimit(1)
            self.YAxisTab.setLimit(0)
            self.YAxisTab.setLimit(1)
            self.Y2AxisTab.setLimit(0)
            self.Y2AxisTab.setLimit(1)
            self.blockSignals(False)

    def repr(self):
        """
        Return the script text.
        """

        output = []
        imports = []
        for tab in [self.XAxisTab, self.YAxisTab, self.Y2AxisTab]:
            out, imp = tab.repr()
            output += out
            imports += imp
        return output, imports


def main(filenames):

    from ..PostprocessorViewer import PostprocessorViewer
    from .FigurePlugin import FigurePlugin
    from .PostprocessorSelectPlugin import PostprocessorSelectPlugin
    import mooseutils

    import matplotlib
    matplotlib.rcParams["figure.figsize"] = (6.25, 6.25)
    matplotlib.rcParams["figure.dpi"] = (100)

    widget = PostprocessorViewer(mooseutils.PostprocessorReader, timeout=None, plugins=[FigurePlugin, AxisTabsPlugin, PostprocessorSelectPlugin])
    widget.onSetFilenames(filenames)
    control = widget.currentWidget().AxisTabsPlugin
    window = widget.currentWidget().FigurePlugin
    window.setFixedSize(QtCore.QSize(625, 625))
    widget.show()

    return control, widget, window

if __name__ == '__main__':

    app = QtWidgets.QApplication(sys.argv)
    control, widget, window = main(['../../../tests/input/white_elephant_jan_2016.csv'])
    sys.exit(app.exec_())
