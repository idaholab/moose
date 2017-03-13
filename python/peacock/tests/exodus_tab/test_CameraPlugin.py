#!/usr/bin/env python
import sys
import unittest
import vtk
from PyQt5 import QtWidgets
from peacock.ExodusViewer.plugins.CameraPlugin import main
from peacock.utils import Testing

class TestCameraPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.initialize([self._filename])
        camera = vtk.vtkCamera()
        camera.SetViewUp(0.2152, 0.4770, 0.8522)
        camera.SetPosition(22.5359, -61.7236, 28.9816)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

    def testFillScreen(self):
        """
        Test 'Fill Screen' button
        """
        self.assertImage('testInitial.png')
        self._widget.CameraPlugin.FillScreenButton.clicked.emit()
        self.assertImage('testFillScreen.png')

    def testReset(self):
        """
        Test 'Reset' button
        """
        self._widget.CameraPlugin.ResetButton.clicked.emit()
        self.assertImage('testReset.png')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
