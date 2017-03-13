import sys
import math
from PyQt5 import QtCore, QtWidgets
from ExodusPlugin import ExodusPlugin

class CameraPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Widget for adjusting the camera.
    """
    windowRequiresUpdate = QtCore.pyqtSignal()

    def __init__(self):
        super(CameraPlugin, self).__init__()

        self.setTitle('Camera')
        self.MainLayout = QtWidgets.QHBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.MainLayout)

        self.FillScreenButton = QtWidgets.QPushButton('Fill Screen')
        self.ResetButton = QtWidgets.QPushButton('Reset')

        self.MainLayout.addStretch()
        self.MainLayout.addWidget(self.FillScreenButton)
        self.MainLayout.addWidget(self.ResetButton)
        self.setup()

    def _setupFillScreenButton(self, qobject):
        """
        Setup method for 'Fill Screen' button.
        """
        qobject.clicked.connect(self._callbackFillScreenButton)

    def _callbackFillScreenButton(self):
        """
        Resets the camera.
        """
        self._result.getVTKRenderer().ResetCameraClippingRange()
        self._result.getVTKRenderer().ResetCamera()
        self._result.setNeedsUpdate(True)
        self.windowRequiresUpdate.emit()

    def _setupResetButton(self, qobject):
        """
        Setup method for the 'Reset' button.
        """
        qobject.clicked.connect(self._callbackResetButton)

    def _callbackResetButton(self):
        """
        Recomputes the original view.
        """
        renderer = self._result.getVTKRenderer()
        renderer.ResetCamera()
        camera = renderer.GetActiveCamera()
        fp = camera.GetFocalPoint()
        p = camera.GetPosition()
        dist = math.sqrt( (p[0]-fp[0])**2 + (p[1]-fp[1])**2 + (p[2]-fp[2])**2 )
        camera.SetPosition(fp[0], fp[1], fp[2]+dist)
        camera.SetViewUp(0.0, 1.0, 0.0)
        self._result.setNeedsUpdate(True)
        self.windowRequiresUpdate.emit()


def main(size=None):
    """
    Run the CameraPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), CameraPlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window = main()
    widget.initialize([filename])
    sys.exit(app.exec_())
