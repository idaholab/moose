import sys
from PyQt5 import QtWidgets, QtCore
from ExodusPlugin import ExodusPlugin
import mooseutils
import peacock

class MediaControlPlugin(QtWidgets.QGroupBox, peacock.base.MediaControlWidgetBase, ExodusPlugin):
    """
    Widget for controlling time of Exodus result.
    """

    #: Emitted when the window needs to be updated.
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: Emitted when the time has changed.
    timeChanged = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the chigger objects options are changed
    readerOptionsChanged = QtCore.pyqtSignal(dict)

    def __init__(self):
        super(MediaControlPlugin, self).__init__()
        self.setMainLayoutName('RightLayout')
        self.setSizePolicy(QtWidgets.QSizePolicy.MinimumExpanding, QtWidgets.QSizePolicy.Minimum)
        self.setup()

    def onWindowCreated(self, *args):
        """
        Initialize the time settings when results are first created.
        """
        super(MediaControlPlugin, self).onWindowCreated(*args)
        self.onWindowUpdated()

    def onWindowUpdated(self):
        """
        Re-initializes the controls for a reader/result object.
        """
        try:
            if self._reader:
                self._times = self._reader.getTimes()
                self._num_steps = len(self._times)
                self._current_step = self._reader.getTimeData().timestep
                self.updateTimeDisplay()
                self.timeChanged.emit()
        except:
            mooseutils.mooseDebug('Failed to update window.', traceback=True, color='RED')

    def updateControls(self, **kwargs):
        """
        Update the current timestep and pass options on to the reader.
        """
        #super(MediaControlPlugin, self).updateControls()
        if 'timestep' in kwargs:
            timestep = kwargs.get('timestep', self._current_step)
            if timestep == (self._num_steps - 1):
                timestep = -1
            self.readerOptionsChanged.emit({'timestep':timestep})

        if 'time' in kwargs:
            self.readerOptionsChanged.emit({'time':kwargs.get('time')})

        self.windowRequiresUpdate.emit()
        self.timeChanged.emit()

def main(size=None):
    """
    Run the MediaControlPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    from peacock.ExodusViewer.plugins.ClipPlugin import ClipPlugin # needed to test that media play disables other plugins
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), MediaControlPlugin, ClipPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.initialize([filename])
    sys.exit(app.exec_())
