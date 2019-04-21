#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
from PyQt5 import QtCore, QtWidgets
from .LineSettingsWidget import LineSettingsWidget
import peacock
import mooseutils


class LineGroupWidget(peacock.base.MooseWidget, QtWidgets.QGroupBox):
    """
    A GroupBox containing the artist toggles for each postprocessor in the supplied data object.

    Args:
        data[PostprocessorDataWidget]: The data object for which toggles will be created.
        cycle[itertools.product]: An iterable container with linestyle and color
        args: Arguments passed to the QWidget object.

    Kwargs:
        cycle[itertools.product]: The style, color pairings to use instead of default (for testing, see test_ArtistGroupWidget)
        key, value pairs are passed to the MooseWiget object.
    """

    #: pyqtSignal: Emitted when the widget has been initialized (used for updating geometry in PostprocessorSelectWidget)
    initialized = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when plot is refreshed
    variablesChanged = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the axes needs to be update
    axesModified = QtCore.pyqtSignal()

    def __init__(self, axes, data, cycle, *args, **kwargs):
        super(LineGroupWidget, self).__init__(**kwargs)
        QtWidgets.QWidget.__init__(self, *args)

        # Extract the line style/color cycle (for testing)
        self._cycle = cycle

        # Store the data and initialize the storage of the toggles to be created.
        self._axes = axes
        self._data = data
        self._toggles = collections.OrderedDict() # maintains order with widget creation, so variable labels remain in order
        self._artists = dict() # artist storage to allow removing lines
        self._initialized = False
        self._time = None # The time index to extract

        # Setup this QGroupBox
        self.setTitle(data.filename())
        self.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
        self.setFixedWidth(510)

        # The layout to which the toggles will be added
        self.MainLayout = QtWidgets.QVBoxLayout(self)
        self.MainLayout.setContentsMargins(5, 0, 0, 0)
        self.MainLayout.setSpacing(0)
        self.setLayout(self.MainLayout)

        # No data moose
        self.NoDataMessage = QtWidgets.QLabel('\nNo data currently available, this will update automatically.\n')

        # Builds the primary axis/variable controls
        self.AxisSelectLayout = QtWidgets.QHBoxLayout()
        self.AxisSelectLayout.setSpacing(10)
        self.AxisSelectLayout.setContentsMargins(0, 0, 0, 0)
        self.AxisVariableLabel = QtWidgets.QLabel('Primary Variable:')
        self.AxisVariable = QtWidgets.QComboBox()
        self.AxisVariable.setFocusPolicy(QtCore.Qt.StrongFocus)

        self.AxisSelectLayout.addWidget(self.AxisVariableLabel)
        self.AxisSelectLayout.addWidget(self.AxisVariable)
        self.AxisSelectLayout.addStretch(1)

        # Add the primary axis/variable controls to the main layouts
        self.MainLayout.addLayout(self.AxisSelectLayout)
        self.MainLayout.addWidget(self.NoDataMessage)

        # Do not show anything until initialized
        self.AxisVariableLabel.setVisible(False)
        self.AxisVariable.setVisible(False)

        # Connect data reload timer
        self._data.dataChanged.connect(self.onDataChanged)

        # Call the update (this will do nothing if data is not yet available and initialize if it is)
        self.plot()

    def showEvent(self, *args):
        """
        Try to load the data when the widget appears.
        """
        self._data.load()

    @QtCore.pyqtSlot()
    def onDataChanged(self):
        """
        Slot called when the dataChanged signal is emitted from the data widget.
        """
        self.plot()

    def clear(self):
        """
        Clears all lines created by this widget.
        """
        for artists in self._artists.itervalues():
            for artist in artists:
                artist.remove()
                del artist
        self._artists.clear()

    def plot(self, time=None):
        """
        Updates the plot with the select lines, for the current time.

        To update the time, use setTime()
        """

        # Update the time if supplied
        if time != None:
            self._time = time

        # Do nothing if data does not exist
        if not self._data:
            self._reset()
            self.axesModified.emit()
            return

        # Perform initialization
        if not self._initialized:
            self._initialize()

        # Remove existing plots
        self.clear()

        # Plot nothing if the time is not valid
        times = self._data.times()
        if (self._time != None) and (times != []) and (self._time not in times):
            self.setEnabled(False)

        # Plot the lines
        else:
            self.setEnabled(True)

            # Extract the primary variable data
            x_var = self.AxisVariable.currentText()
            x = self._data(x_var)

            # Loop through all the line settings toggles and create lines
            y_vars = [[], []]
            for variable, toggle in self._toggles.iteritems():
                if toggle.isValid():
                    settings = toggle.settings()
                    i = settings.pop('axis')
                    y_vars[i].append(variable)
                    y = self._data(variable, time=self._time, warning=False)
                    if self._axes[i]:
                        self._artists[variable] = self._axes[i].plot(x, y, **settings)

        # Emit variable names
        self.variablesChanged.emit()

        # Re-draw the figure
        self.axesModified.emit()

    def getAxisLabels(self):
        """
        Return the active x,y axis labels.
        """

        # x
        x_var = self.AxisVariable.currentText()

        y_vars = []
        y2_vars = []
        for variable, toggle in self._toggles.iteritems():
            if toggle.isValid():
                if toggle.axis() == 'right':
                    y2_vars.append(variable)
                else:
                    y_vars.append(variable)

        return x_var, y_vars, y2_vars

    def isValid(self):
        """
        Returns True if any lines are active.
        """
        return any([toggle.isValid() for toggle in self._toggles.itervalues()])

    def repr(self):
        """
        Outputs data for creating python script.

        see PostprocessorPlotWidget
        """

        # Do nothing if no data is selectd
        if not any([toggle.isValid() for toggle in self._toggles.itervalues()]):
            return [], []

        # Read the data
        output, imports = self._data.repr()

        # Get x-axis data
        if self._time:
            output += ['x = data({}, time={})'.format(repr(str(self.AxisVariable.currentText())), repr(self._time))]
        else:
            output += ['x = data({})'.format(repr(str(self.AxisVariable.currentText())))]

        # Plot the results
        for toggle in self._toggles.itervalues():
            if toggle.isValid():
                out, imp = toggle.repr(time=self._time)
                output += ['']
                output += out
                imports += imp

        return output, imports

    def _reset(self):
        """
        Resets the state of the widget to pre-initialized, so if data disappears so does the plot.
        """
        # Clear the plot
        self.clear()

        # Clear the widgets
        for toggle in self._toggles.itervalues():
            toggle.setVisible(False) # If I don't do this, there is a ghosted image of the widget hanging around
            self.MainLayout.removeWidget(toggle)
            toggle.setParent(None)
        self._toggles.clear()

        # Show "No data" message
        self.AxisVariableLabel.setVisible(False)
        self.AxisVariable.setVisible(False)
        self.NoDataMessage.setVisible(True)

        # Emit empty axis variable names
        self.variablesChanged.emit()

        # Enable re-initialization
        self._initialized = False

    def _initialize(self, create=True):
        """
        Creates LineSettingsWidget for postprocessor data. (protected)
        """

        # Enabled the widget
        self.NoDataMessage.setVisible(False)
        self.AxisVariable.setVisible(True)
        self.AxisVariableLabel.setVisible(True)

        if create:
            # Create a toggle control for each piece of data
            for var in self._data.variables():
                style, color = next(self._cycle, ('-', [0, 0, 0]))
                toggle = LineSettingsWidget(var, linestyle=style, color=color)
                toggle.clicked.connect(self.plot)
                self.MainLayout.addWidget(toggle)
                self._toggles[var] = toggle

        # The widget is initialized after all the toggles have been added, so it is time to finish the setup
        self._initialized = True
        self.setup()

        # Emit the initialization signal
        self.initialized.emit()

    def _setuNoDataMessage(self, qobject):
        """
        Setup method for no data label.
        """
        qobject.setText()


    def _setupAxisVariable(self, qobject):
        """
        Setup method for primary axis variable selection.
        """
        qobject.currentIndexChanged.connect(self._callbackAxisVariable)

        for var in self._data.variables():
            qobject.addItem(var)

    def _callbackAxisVariable(self):
        """
        Callback for primary variable selection.
        """
        var = self.AxisVariable.currentText()
        for toggle in self._toggles.itervalues():
            toggle.setEnabled(True)
        self._toggles[var].setEnabled(False)
        self.clear()
        self.plot()

    def filename(self):
        """
        Just get the filename of the data
        """
        return self._data.filename()

    def sameData(self, d):
        """
        Just returns a bool on whether the incoming data has the same
        variables as the current variables.
        """
        variable_names = [str(v) for v in d.variables()]
        return sorted(self._toggles.keys()) == sorted(variable_names)

    def setData(self, axes, d):
        """
        Set new data, keeping all the current LineSettingsWidget
        """
        self.clear()
        self._data.disconnect()
        self._data = d
        self._axes = axes
        self._data.dataChanged.connect(self.onDataChanged)
        self.axesModified.emit()
        self.plot()

def main(data, pp_class=mooseutils.VectorPostprocessorReader):
    """
    Create widgets for running LineGroupWidget
    """
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    from FigurePlugin import FigurePlugin
    import matplotlib.pyplot as plt
    import numpy as np
    import itertools

    # Create main widget
    widget = PostprocessorViewer(plugins=[FigurePlugin])
    widget.onSetFilenames(['empty_file'])
    layout = widget.currentWidget().LeftLayout
    window = widget.currentWidget().FigurePlugin
    window.setFixedSize(QtCore.QSize(625, 625))

    # Create LineGroupWidget
    cycle = itertools.product(['-', '--', '-.', ':'], plt.cm.Paired(np.linspace(0, 1, 11)))
    control = LineGroupWidget(window.axes(), data, cycle)
    layout.addWidget(control)
    control.axesModified.connect(window.onAxesModified)

    def axis_label():
        """
        A labeling function for setting axis labels.
        """
        x,y,y2 = control.getAxisLabels()
        control._axes[0].set_xlabel(x)
        control._axes[0].set_ylabel('; '.join(y))
        control._axes[1].set_ylabel('; '.join(y2))

    control.variablesChanged.connect(axis_label)

    widget.show()

    return control, widget, window


if __name__ == '__main__':
    import sys
    import mooseutils
    from peacock.PostprocessorViewer.PostprocessorDataWidget import PostprocessorDataWidget

    app = QtWidgets.QApplication(sys.argv)
    filename = '../../../tests/input/white_elephant_jan_2016.csv'
    reader = mooseutils.PostprocessorReader(filename)
    data = PostprocessorDataWidget(reader)
    control, widget, window = main(data)
    sys.exit(app.exec_())
