import inspect
import subprocess
import os
from PyQt5 import QtCore, QtWidgets
from ExodusPlugin import ExodusPlugin
from VTKWindowPlugin import VTKWindowPlugin
import mooseutils
import chigger

class ExternalVTKWindowPlugin(VTKWindowPlugin):
    """
    VTK window for external gold/diff use.
    """

    def __init__(self, toggle, size=None, text=None):
        super(ExternalVTKWindowPlugin, self).__init__(size=size)

        self.setWindowFlags(QtCore.Qt.SubWindow | QtCore.Qt.CustomizeWindowHint | QtCore.Qt.WindowTitleHint | QtCore.Qt.WindowMinMaxButtonsHint | QtCore.Qt.WindowCloseButtonHint)
        self._widget_size = None

        # The toggle button that controls the window
        self._toggle = toggle

        # Add text annotation
        self._text = None
        if text:
            self._text = chigger.annotations.TextAnnotation(text=text, position=[0.011, 0.01], layer=2, text_color=[1, 0.84, 0], font_size=48)

    def onJobStart(*args):
        """
        Ignores the job start time.
        """
        pass

    def onFileChanged(self, *args):
        """
        Adds text to window, if provided.
        """
        super(ExternalVTKWindowPlugin, self).onFileChanged(*args)
        if self._text:
            self._window.append(self._text)

    def sizeHint(self, *args):
        """
        Return the saved size.
        """
        if self._widget_size:
            return self._widget_size
        else:
            return super(ExternalVTKWindowPlugin, self).size()

    def closeEvent(self, *args):
        """
        Store the size of the window.
        """
        self._widget_size = self.size()
        self._toggle.setCheckState(QtCore.Qt.Unchecked)
        self._toggle.clicked.emit(False)
        #super(ExternalVTKWindowPlugin, self).closeEvent(*args)


class GoldDiffPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Plugin for toggling the Gold/Diff VTK windows.
    """
    windowRequiresUpdate = QtCore.pyqtSignal()

    def __init__(self, size=None):
        super(GoldDiffPlugin, self).__init__()

        self.setTitle('Compare Files:')
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.setLayout(self.MainLayout)

        self.GoldToggle = QtWidgets.QCheckBox("Gold")
        self.DiffToggle = QtWidgets.QCheckBox("Exodiff")
        self.LinkToggle = QtWidgets.QCheckBox("Link Camera(s)")

        self.MainLayout.addWidget(self.GoldToggle)
        self.MainLayout.addWidget(self.DiffToggle)
        self.MainLayout.addWidget(self.LinkToggle)

        self.GoldVTKWindow = ExternalVTKWindowPlugin(self.GoldToggle, size=size, text='GOLD')
        self.DiffVTKWindow = ExternalVTKWindowPlugin(self.DiffToggle, size=size, text='EXODIFF')

        self.setup()

        # Locate MOOSE exodiff program
        self._exodiff = None
        moose_dir = os.path.abspath(os.path.join(os.path.realpath(inspect.getfile(self.__class__)), '..', '..', '..'))
        exodiff = os.path.join(os.getenv('MOOSE_DIR', moose_dir), 'framework', 'contrib', 'exodiff', 'exodiff')
        if os.path.isfile(exodiff):
            self._exodiff = exodiff

    def initialize(self, *args):
        """
        Initialize this widget.

        All plugins are created at this point, so the link camera button can be connect if the VTKWindowPlugin
        is available on the parent.
        """
        super(GoldDiffPlugin, self).initialize()

        # Enable/disable the link camera toggle based on the existence of the main window.
        if hasattr(self.parent(), 'VTKWindowPlugin'):
            self.LinkToggle.clicked.connect(self._callbackLinkToggle)
            self.LinkToggle.setChecked(True)
            self.LinkToggle.clicked.emit(True)
        else:
            self.LinkToggle.setEnabled(False)

        # Disable exodiff button if program is not available
        self.DiffToggle.setEnabled(bool(self._exodiff))

    def onPlayStop(self):
        """
        Need to override this from ExodusPlugin to make sure the we don't
        get enabled by accident.
        """
        self.setActive()

    def setActive(self):
        """
        Sets the Qt enabled status for this widget.
        """
        value = bool(mooseutils.gold(self._filename) != None)
        self.setEnabled(value)
        if value:
            self._callbackGoldToggle(self.GoldToggle.isChecked())
            self._callbackDiffToggle(self.DiffToggle.isChecked())
        else:
            self._callbackGoldToggle(False)
            self._callbackDiffToggle(False)

    def onFileChanged(self, filename):
        """
        Enables/disables this plugin based on the existence of a gold files.
        """
        self._filename = filename
        self.load(self._filename, 'Filename')
        self.setActive()

    def onWindowCreated(self, *args):
        """
        Change default enable/disable behavior this is handled by onFileChanged.
        """
        super(GoldDiffPlugin, self).onWindowCreated(*args)
        self.setEnabled(False)

    def onWindowUpdated(self):
        """
        Update the gold/diff window settings.
        """
        for window in [self.GoldVTKWindow, self.DiffVTKWindow]:
            if window.isVisible():
                window.onReaderOptionsChanged(self._reader.options())
                window.onResultOptionsChanged(self._result.options())
                window.onWindowOptionsChanged(self._window.options())
                window.onResultOptionsChanged({'range':None})
                window.onWindowRequiresUpdate()

    def _setupGoldToggle(self, qobject):
        """
        The setup method for GoldToggle widget.

        Args:
            qobject: The widget being setup.
        """
        qobject.clicked.connect(self._callbackGoldToggle)

    def _callbackGoldToggle(self, value):
        """
        Callback for GoldToggle widget.

        Args:
            value[bool]: True/False indicating the toggle state of the widget.
        """
        self.store(self._filename, 'Filename')
        if value:
            self.GoldVTKWindow.show()
            self.GoldVTKWindow.blockSignals(True)
            self.GoldVTKWindow.onFileChanged(self._filename)
            self.GoldVTKWindow.onCameraChanged(self._result.getVTKRenderer().GetActiveCamera())
            self.GoldVTKWindow.blockSignals(False)
            self.onWindowUpdated()
        else:
            self.GoldVTKWindow.hide()

    def _setupDiffToggle(self, qobject):
        """
        The setup method for DiffToggle widget.

        Args:
            qobject: The widget being setup.
        """
        qobject.clicked.connect(self._callbackDiffToggle)

    def _callbackDiffToggle(self, value):
        """
        Callback for DiffToggle widget.

        Args:
            value[bool]: True/False indicating the toggle state of the widget.
        """
        self.store(self._filename, 'Filename')

        if value:
            goldname = mooseutils.gold(self._filename)
            diffname = 'peacock_tmp_diff.exo'

            if not os.path.exists(goldname):
                mooseutils.mooseError('The gold file was not located: {}'.format(goldname))

            cmd = [self._exodiff, '-map', '-F', '1e-10', '-t', '5.5e-06', os.path.abspath(self._filename), os.path.abspath(goldname), os.path.abspath(diffname)]
            subprocess.call(cmd)

            if os.path.exists(diffname):
                self.DiffVTKWindow.blockSignals(True)
                self.DiffVTKWindow.setVisible(True)
                self.DiffVTKWindow.onFileChanged(diffname)
                self.DiffVTKWindow.onCameraChanged(self._result.getVTKRenderer().GetActiveCamera())
                self.DiffVTKWindow.blockSignals(False)
                self.onWindowUpdated()
            else:
                mooseutils.mooseError('Failed to generate Exodiff file with command:\n{}'.format(' '.join(cmd)))

        else:
            self.DiffVTKWindow.setVisible(False)

    def _callbackLinkToggle(self, value):
        """
        Connect/disconnect the cameras between windows.

        NOTE: This doesn't get called (b/c the button is disabled) if VTKWindowPlugin does not exist on parent.
        see initialization
        """
        self.store(self._filename, 'Filename')
        master = self.parent().VTKWindowPlugin
        slaves = [self.GoldVTKWindow, self.DiffVTKWindow]
        if value:
            for slave in slaves:
                master.cameraChanged.connect(slave.onCameraChanged)
                slave.cameraChanged.connect(master.onCameraChanged)
                for slave2 in slaves:
                    slave2.cameraChanged.connect(slave.onCameraChanged)
        else:
            for slave in slaves:
                master.cameraChanged.disconnect(slave.onCameraChanged)
                slave.cameraChanged.disconnect(master.onCameraChanged)
                for slave2 in slaves:
                    slave2.cameraChanged.disconnect(slave.onCameraChanged)


def main(size=None):
    """
    Run the VTKFilePlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from VTKWindowPlugin import VTKWindowPlugin
    from FilePlugin import FilePlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin, lambda: GoldDiffPlugin(size=size)])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    import sys
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget, window = main()
    widget.initialize(filenames)
    window.onResultOptionsChanged({'variable':'diffused'})
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
