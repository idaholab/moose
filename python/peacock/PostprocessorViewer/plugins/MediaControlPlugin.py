#!/usr/bin/env python
import sys
import bisect
from PyQt5 import QtWidgets, QtCore
from PostprocessorPlugin import PostprocessorPlugin
import mooseutils
import peacock

class MediaControlPlugin(QtWidgets.QGroupBox, peacock.base.MediaControlWidgetBase, PostprocessorPlugin):
    """
    Time controls for Postprocessor data.

    Args:
        date[list]: A list of PostprocessorDataWidget objects.
    """

    #: pyqtSignal: Emits when the time is changed.
    timeChanged = QtCore.pyqtSignal(float)

    def __init__(self):
        super(MediaControlPlugin, self).__init__()
        self.setEnabled(False)
        self.setMainLayoutName('RightLayout') # used by plugin system to place widget
        self._data = []

    @QtCore.pyqtSlot()
    def onDataChanged(self):
        """
        Slot called when the dataChanged signal is emitted from the data widget.
        """
        try:
            self.initialize(self._data)
        except:
            pass

    def initialize(self, data):
        """
        Initializes the time controls for the current data.
        """
        super(MediaControlPlugin, self).initialize(data)

        # Update data if provided
        if data != None:
            for d in self._data:
                d.dataChanged.disconnect(self.onDataChanged)
            self._data = data

        # The time information
        self._times = []

        # Loop through the data objects
        for data in self._data:
            data.dataChanged.connect(self.onDataChanged)
            self._times += data.times()

        # Remove duplicates and sort the times
        self._times = sorted(list(set(self._times)))

        # Set the number of steps
        self._num_steps = len(self._times)

        # Enable the controls and update them
        if self._times:
            self.setVisible(True)
            self.updateControls()
        else:
            self.setVisible(False)

    def updateControls(self, **kwargs):
        """
        Updates the time/timestep widgets and data for the current step.
        """
        self._current_step = kwargs.pop('timestep', self._current_step)

        time = kwargs.pop('time', None)
        if time != None:
            idx = bisect.bisect_right(self._times, time) - 1
            if idx < 0:
                idx = 0
            elif idx > len(self._times)-1:
                idx = -1
            self._current_step = idx

        self.updateTimeDisplay()
        self.timeChanged.emit(self._times[self._current_step])

def main(filenames):
    from peacock.PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
    from FigurePlugin import FigurePlugin
    from PostprocessorSelectPlugin import PostprocessorSelectPlugin
    widget = PostprocessorViewer(mooseutils.VectorPostprocessorReader, plugins=[FigurePlugin, PostprocessorSelectPlugin, MediaControlPlugin])
    widget.initialize(filenames)
    widget.show()
    return widget

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    widget = main(['../../../tests/input/vpp_*.csv', '../../../tests/input/vpp2_*.csv'])
    app.exec_()
