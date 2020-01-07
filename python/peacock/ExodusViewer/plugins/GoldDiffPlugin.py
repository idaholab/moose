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
import vtk
from PyQt5 import QtCore, QtWidgets
from .ExodusPlugin import ExodusPlugin
from .VTKWindowPlugin import VTKWindowPlugin
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
        self.DiffVTKWindow = None#ExternalVTKWindowPlugin(self.DiffToggle, size=size)

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

    def _loadPlugin(self):
        """
        Loads plugin state.
        """
        self.load(self.GoldToggle)
        self.load(self.DiffToggle)
        self.load(self.LinkToggle)

    def onSetVariable(self, *args):
        """
        Update variable for open Gold/Diff windows.
        """
        super(GoldDiffPlugin, self).onSetVariable(*args)
        if self.hasGoldWindow():
            self.GoldVTKWindow.onSetVariable(*args)
        if self.hasDiffWindow():
            self.DiffVTKWindow.onSetVariable(*args)

    def onSetComponent(self, *args):
        """
        Update component for open Gold/Diff windows.
        """
        super(GoldDiffPlugin, self).onSetComponent(*args)
        if self.hasGoldWindow():
            self.GoldVTKWindow.onSetComponent(*args)
        if self.hasDiffWindow():
            self.DiffVTKWindow.onSetComponent(*args)

    def onReaderOptionsChanged(self, options):
        """
        Pass on the reader options to the gold/diff window(s).
        """
        self.updateOptions()
        if self.hasGoldWindow():
            self.GoldVTKWindow.onReaderOptionsChanged(options)
        if self.hasDiffWindow():
            self.DiffVTKWindow.onReaderOptionsChanged(options)

    def onResultOptionsChanged(self, options):
        """
        Pass on the result options to the gold/diff window(s).
        """
        self.updateOptions()
        if self.hasGoldWindow():
            self.GoldVTKWindow.onResultOptionsChanged(options)
        if self.hasDiffWindow():
            self.DiffVTKWindow.onResultOptionsChanged(options)

    def onWindowOptionsChanged(self, options):
        """
        Pass on the window options to the gold/diff window(s).
        """
        self.updateOptions()
        if self.hasGoldWindow():
            self.GoldVTKWindow.onWindowOptionsChanged(options)
        if self.hasDiffWindow():
            self.DiffVTKWindow.onWindowOptionsChanged(options)

    def onCameraChanged(self, *args):
        """
        Slot for when camera is changed.
        """
        link = self.LinkToggle.isChecked()
        if link and self.hasGoldWindow():
            self.GoldVTKWindow.onCameraChanged(*args)
        if link and self.hasDiffWindow():
            self.DiffVTKWindow.onCameraChanged(*args)

    def hasGoldWindow(self):
        """
        Return True if the Gold window is open.
        """
        return self.GoldToggle.isChecked() and self.GoldVTKWindow.isVisible()

    def hasDiffWindow(self):
        """
        Return True if the Diff window is open.
        """
        diff = self.DiffToggle.isChecked() if self._exodiff else False
        return diff and self.DiffVTKWindow.isVisible()

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
        gold = self.GoldToggle.isChecked() if self.GoldVTKWindow else False
        goldname = mooseutils.gold(self._filename)
        if gold and (not self.GoldVTKWindow.isVisible()):
            self.GoldVTKWindow.show()
            self.GoldVTKWindow.onSetFilename(goldname)
            self.GoldVTKWindow.onSetVariable(self._variable)
            self.GoldVTKWindow.onSetComponent(self._component)
            self.GoldVTKWindow.onWindowRequiresUpdate()
        elif (not gold) and self.GoldVTKWindow and self.GoldVTKWindow.isVisible():
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
        self.store(self.GoldToggle)#, key=(self._filename, None, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupDiffToggle(self, qobject):
        """
        The setup method for DiffToggle widget.

        Args:
            qobject: The widget being setup.
        """
        self.DiffToggle.setEnabled(bool(self._exodiff))
        if self._exodiff:
            qobject.clicked.connect(self._callbackDiffToggle)

    def _callbackDiffToggle(self, value):
        """
        Callback for DiffToggle widget.

        Args:
            value[bool]: True/False indicating the toggle state of the widget.
        """
        self.store(self.DiffToggle)#, key=(self._filename, None, None))
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
        self.store(self.LinkToggle)#, key=(self._filename, None, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

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
    from ..ExodusPluginManager import ExodusPluginManager
    from .VTKWindowPlugin import VTKWindowPlugin
    from .FilePlugin import FilePlugin
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
