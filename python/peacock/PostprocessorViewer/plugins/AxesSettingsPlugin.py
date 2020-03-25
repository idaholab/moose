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
import peacock
from .PostprocessorPlugin import PostprocessorPlugin

class AxesSettingsPlugin(QtWidgets.QGroupBox, PostprocessorPlugin):
    """
    Widget for controlling global axes settings.

    Args:
        axes[pyplot.Axes]: The axes object to apply the settings.
        args[tuple]: Passed to QtWidgets.QGroupBox widget

    Kwargs:
        key, value pairs are passed to MooseWidget object.
    """

    #: list: List of all possible legend locations
    legend_loc = ['best', 'upper right', 'upper left', 'lower left', 'lower right', 'right', 'center left', 'center right', 'lower center', 'upper center', 'center']

    #: pyqtSingal: Should be emitted when the axes have been modified.
    axesModified = QtCore.pyqtSignal()

    def __init__(self, *args, **kwargs):
        peacock.base.MooseWidget.__init__(self)
        QtWidgets.QWidget.__init__(self, *args)
        PostprocessorPlugin.__init__(self, **kwargs)

        self.MainLayout = QtWidgets.QVBoxLayout()
        self.setLayout(self.MainLayout)


        # Title
        self.TitleLayout = QtWidgets.QHBoxLayout()
        self.TitleLabel = QtWidgets.QLabel('Title:')
        self.Title = QtWidgets.QLineEdit()
        self.TitleLayout.addWidget(self.TitleLabel)
        self.TitleLayout.addWidget(self.Title)

        # Legend Toggles
        self.LegendLayout = QtWidgets.QGridLayout()
        self.Legend = QtWidgets.QCheckBox('Legend (Left) ')
        self.LegendLocation = QtWidgets.QComboBox()

        self.Legend2 = QtWidgets.QCheckBox('Legend (Right)')
        self.Legend2Location = QtWidgets.QComboBox()

        self.LegendLayout.addWidget(self.Legend, 0, 0)
        self.LegendLayout.addWidget(self.LegendLocation, 0, 1)

        self.LegendLayout.addWidget(self.Legend2, 1, 0)
        self.LegendLayout.addWidget(self.Legend2Location, 1, 1)

        self.MainLayout.addLayout(self.TitleLayout)
        self.MainLayout.addLayout(self.LegendLayout)

        self.setup()

    def onSetData(self, data):
        """
        Initialize the widget.
        """
        self.onAxesModified()

    @QtCore.pyqtSlot()
    def onAxesModified(self):
        """
        Updates the Axes with the settings from this widget.
        """

        # Legend helper function
        def setup_legend(box, loc, axes):
            has_data = axes.has_data()
            box.setEnabled(has_data)
            checked = box.isChecked()
            loc.setEnabled(checked and has_data)

            if has_data and checked:

                legend = axes.legend(loc=loc.currentText())
                legend.set_visible(True)
                return

            legend = axes.get_legend()
            if legend:
                legend.set_visible(False)

        # Legends
        setup_legend(self.Legend, self.LegendLocation, self.axes(0))
        setup_legend(self.Legend2, self.Legend2Location, self.axes(1))

        # Title
        self.axes(0).set_title(self.Title.text())
        self.axesModified.emit()

    def repr(self):
        """
        Returns a representation of this widget for use in python script.
        """

        output = []
        if self.Legend.isChecked():
            loc = self.legend_loc[self.LegendLocation.currentIndex()]
            output += ["axes0.legend(loc={})".format(repr(loc))]

        if self.Legend2.isChecked():
            loc = self.legend_loc[self.Legend2Location.currentIndex()]
            output += ["axes1.legend(loc={})".format(repr(loc))]

        title = str(self.Title.text())
        if title:
            output += ["axes0.set_title({})".format(repr(title))]

        if output:
            output.insert(0, '\n# Axes Settings')

        return output, []

    def _setupTitle(self, qobject):
        """
        Setup method for Title widget.
        """
        qobject.editingFinished.connect(self.onAxesModified)

    def _setupLegend(self, qobject):
        """
        Setup method for left-hand legend toggle widget.
        """
        qobject.clicked.connect(self.onAxesModified)

    def _setupLegend2(self, qobject):
        """
        Setup method of right-hand legend.
        """
        qobject.clicked.connect(self.onAxesModified)

    def _setupLegendLocation(self, qobject):
        """
        Setup method for legend (left) position widget.
        """
        qobject.setEnabled(False)
        for loc in self.legend_loc:
            qobject.addItem(loc)
        qobject.currentIndexChanged.connect(self.onAxesModified)

    def _setupLegend2Location(self, qobject):
        """
        Setup method for legend (right) position widget.
        """
        qobject.setEnabled(False)
        for loc in self.legend_loc:
            qobject.addItem(loc)
        qobject.currentIndexChanged.connect(self.onAxesModified)

def main(filenames):

    from ..PostprocessorViewer import PostprocessorViewer
    from .FigurePlugin import FigurePlugin
    from .PostprocessorSelectPlugin import PostprocessorSelectPlugin
    import mooseutils

    import matplotlib
    matplotlib.rcParams["figure.figsize"] = (6.25, 6.25)
    matplotlib.rcParams["figure.dpi"] = (100)

    widget = PostprocessorViewer(mooseutils.PostprocessorReader, timeout=None, plugins=[FigurePlugin, AxesSettingsPlugin, PostprocessorSelectPlugin])
    widget.onSetFilenames(filenames)
    control = widget.currentWidget().AxesSettingsPlugin
    window = widget.currentWidget().FigurePlugin
    window.setFixedSize(QtCore.QSize(625, 625))
    widget.show()

    return control, widget, window

if __name__ == '__main__':

    app = QtWidgets.QApplication(sys.argv)
    control, widget, window = main(['../../../tests/input/white_elephant_jan_2016.csv'])
    sys.exit(app.exec_())
