#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
import os
from PyQt5 import QtCore, QtWidgets
from ExodusPlugin import ExodusPlugin
from VTKWindowPlugin import VTKWindowPlugin
import mooseutils

class ExternalVTKWindowPlugin(VTKWindowPlugin):
    """
    VTK window for external gold/diff use, it handles storing size and de-selecting main window check boxes.
    """

    def __init__(self, toggle, size=None, text=None):
        super(ExternalVTKWindowPlugin, self).__init__(size=size)

        self.setWindowFlags(QtCore.Qt.SubWindow | QtCore.Qt.CustomizeWindowHint | QtCore.Qt.WindowTitleHint | \
                            QtCore.Qt.WindowMinMaxButtonsHint | QtCore.Qt.WindowCloseButtonHint)
        self._widget_size = None

        # The toggle button that controls the window
        self._toggle = toggle

        # Add text annotation
        self._text = None
        if text:
            self.setWindowTitle(text)

    def onJobStart(*args):
        """
        Ignores the job start time.
        """
        pass

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


class GoldDiffPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Plugin for toggling the Gold/Diff VTK windows.
    """
    windowRequiresUpdate = QtCore.pyqtSignal()
    cameraChanged = QtCore.pyqtSignal(tuple, tuple, tuple)

    def __init__(self, size=None):
        super(GoldDiffPlugin, self).__init__()

        self.MainLayout = QtWidgets.QHBoxLayout(self)

        self.GoldToggle = QtWidgets.QCheckBox("Gold")
        self.DiffToggle = QtWidgets.QCheckBox("Exodiff")
        self.LinkToggle = QtWidgets.QCheckBox("Link Camera(s)")

        self.MainLayout.addWidget(self.GoldToggle)
        self.MainLayout.addWidget(self.DiffToggle)
        self.MainLayout.addWidget(self.LinkToggle)

        self.GoldVTKWindow = ExternalVTKWindowPlugin(self.GoldToggle, size=size, text='GOLD')
        self.DiffVTKWindow = None

        # Locate MOOSE exodiff program
        self._exodiff = None
        moose_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
        exodiff = os.path.join(os.getenv('MOOSE_DIR', moose_dir), 'framework', 'contrib', 'exodiff', 'exodiff')
        if os.path.isfile(exodiff):
            self.DiffVTKWindow = ExternalVTKWindowPlugin(self.DiffToggle, size=size, text='EXODIFF')
            self._exodiff = exodiff

        self.setup()

        self._gold_observer = None
        self._diff_observer = None
        self._main_observer = None
        self._initialized = False

    def _initialize(self, *args, **kwargs):
        """
        Initialize this widget.

        All plugins are created at this point, so the link camera button can be connect if the
        VTKWindowPlugin is available on the parent.
        """

        # Enable/disable the link camera toggle based on the existence of the main window.
        if self._plugin_manager and 'VTKWindowPlugin' in self._plugin_manager:
            self.LinkToggle.clicked.connect(self._callbackLinkToggle)
            self.LinkToggle.setChecked(True)
            self.LinkToggle.clicked.emit(True)
        else:
            self.LinkToggle.setEnabled(False)

        # Disable exodiff button if program is not available
        self.DiffToggle.setEnabled(bool(self._exodiff))
        self._initialized = True

    def onSetVariable(self, variable):
        """
        Loads stored state and the toggles when the variable is changed.

        Notice that the state being loaded is based on the filename, which is what is stored (see callbacks
        below). This is done when the variable is changed rather than filename to ensure that the self._variable
        has the proper value so the gold/diff window is loaded with the correct variable.
        """
        super(GoldDiffPlugin, self).onSetVariable(variable)
        self.load(self.GoldToggle, key=(self._filename, None, None))
        self.load(self.DiffToggle, key=(self._filename, None, None))
        self.load(self.LinkToggle, key=(self._filename, None, None))
        self.updateOptions()

    def onWindowResult(self, *args):
        """
        Initializes the widget on the first render and updates the widget.
        """
        if not self._initialized:
            self._initialize()
        self.updateOptions()

    def onReaderOptionsChanged(self, options):
        """
        Pass on the reader options to the gold/diff window(s).
        """
        self.updateOptions()
        if self.GoldToggle.isChecked():
            self.GoldVTKWindow.onReaderOptionsChanged(options)
        if self.DiffToggle.isChecked():
            self.DiffTKWindow.onReaderOptionsChanged(options)

    def onResultOptionsChanged(self, options):
        """
        Pass on the result options to the gold/diff window(s).
        """
        self.updateOptions()
        if self.GoldToggle.isChecked():
            self.GoldVTKWindow.onResultOptionsChanged(options)
        if self.DiffToggle.isChecked():
            self.DiffTKWindow.onResultOptionsChanged(options)

    def updateOptions(self):
        """
        Control the Gold/Diff VTK windows.
        """
        value = mooseutils.gold(self._filename) is not None
        self.setVisible(value)
        self.setEnabled(value)
        if not value:
            self.GoldToggle.setChecked(False)
            self.DiffToggle.setChecked(False)

        # Gold window toggle
        gold = self.GoldToggle.isChecked()
        goldname = mooseutils.gold(self._filename)
        if gold and (not self.GoldVTKWindow.isVisible()):
            self.GoldVTKWindow.show()
            self.GoldVTKWindow.onSetFilename(goldname)
            self.GoldVTKWindow.onSetVariable(self._variable)
            self.GoldVTKWindow.onSetComponent(self._component)
            self.GoldVTKWindow.onWindowRequiresUpdate()
        elif (not gold) and self.GoldVTKWindow.isVisible():
            self.GoldVTKWindow.hide()

        # Diff Window toggle
        diff = self.DiffToggle.isChecked() if self._exodiff else False
        if diff and (not self.DiffVTKWindow.isVisible()):

            diffname = self._filename + '.diff'
            cmd = [self._exodiff, '-map', '-F', '1e-10', '-t', '5.5e-06',
                   os.path.abspath(self._filename),
                   os.path.abspath(goldname),
                   os.path.abspath(diffname)]
            subprocess.call(cmd)

            self.DiffVTKWindow.show()
            self.DiffVTKWindow.onSetFilename(diffname)
            self.DiffVTKWindow.onSetVariable(self._variable)
            self.DiffVTKWindow.onSetComponent(self._component)
            self.DiffVTKWindow.onWindowRequiresUpdate()

        elif (not diff) and (self.DiffVTKWindow is not None) and self.DiffVTKWindow.isVisible():
            self.DiffVTKWindow.hide()

        # Camera linkage
        link = self.LinkToggle.isChecked()
        if link:
            if gold and (self._gold_observer is None):
                self._gold_observer = self.GoldVTKWindow._window.getVTKInteractor().AddObserver("RenderEvent", self._callbackGoldRenderEvent)

            if diff and (self._diff_observer is None):
                self._diff_observer = self.DiffVTKWindow._window.getVTKInteractor().AddObserver("RenderEvent", self._callbackDiffRenderEvent)

        else:
            if self._gold_observer is not None:
                self.GoldVTKWindow._window.getVTKInteractor().RemoveObserver(self._gold_observer)
                self._gold_observer = None

            if self._diff_observer is not None:
                self.DiffVTKWindow._window.getVTKInteractor().RemoveObserver(self._diff_observer)
                self._diff_observer = None

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
        self.store(self.GoldToggle, key=(self._filename, None, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupDiffToggle(self, qobject):
        """
        The setup method for DiffToggle widget.

        Args:
            qobject: The widget being setup.
        """
        if self._exodiff:
            qobject.clicked.connect(self._callbackDiffToggle)

    def _callbackDiffToggle(self, value):
        """
        Callback for DiffToggle widget.

        Args:
            value[bool]: True/False indicating the toggle state of the widget.
        """
        self.store(self.DiffToggle, key=(self._filename, None, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupLinkToggle(self, qobject):
        """
        Setup the camera link toggling.
        """
        qobject.setCheckState(QtCore.Qt.Checked)
        qobject.clicked.connect(self._callbackLinkToggle)

    def _callbackLinkToggle(self, value):
        """
        Connect/disconnect the cameras between windows.

        NOTE: This doesn't get called (b/c the button is disabled) if VTKWindowPlugin does not exist
        on the plugin manager, see initialization
        """
        self.store(self.LinkToggle, key=(self._filename, None, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def onCameraChanged(self, *args):
        """
        Slot for when camera is changed.
        """
        link = self.LinkToggle.isChecked()
        if link and self.GoldVTKWindow.isVisible():
            self.GoldVTKWindow.onCameraChanged(*args)
        if link and self._exodiff and self.DiffVTKWindow.isVisible():
            self.DiffVTKWindow.onCameraChanged(*args)

    def _callbackGoldRenderEvent(self, *args):
        """
        Called when the gold window RenderEvent occurs.
        """
        camera = self.GoldVTKWindow._result.getVTKRenderer().GetActiveCamera()
        view, position, focal = camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint()
        self.cameraChanged.emit(view, position, focal)

        if self._exodiff and self.DiffVTKWindow.isVisible():
            self.DiffVTKWindow.onCameraChanged(view, position, focal)

    def _callbackDiffRenderEvent(self, *args):
        """
        Called when the diff window RenderEvent occurs.
        """
        camera = self.DiffVTKWindow._result.getVTKRenderer().GetActiveCamera()
        view, position, focal = camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint()
        self.cameraChanged.emit(view, position, focal)

        if self.GoldVTKWindow.isVisible():
            self.GoldVTKWindow.onCameraChanged(view, position, focal)

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
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
