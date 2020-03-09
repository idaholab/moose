#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import unittest
import vtk
from PyQt5 import QtWidgets, QtCore
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
        self._window.onSetFilename(self._filename)
        self._window.onSetVariable('diffused')
        self._window.onWindowRequiresUpdate()

        camera = vtk.vtkCamera()
        camera.SetViewUp(0.2152, 0.4770, 0.8522)
        camera.SetPosition(22.5359, -61.7236, 28.9816)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint())

    def testFillScreen(self):
        """
        Test 'Fill Screen' button
        """
        self._widget.CameraPlugin.FillScreenButton.setChecked(QtCore.Qt.Checked)
        self._widget.CameraPlugin.FillScreenButton.clicked.emit()
        self.assertImage('testFillScreen.png')

    def testReset(self):
        """
        Test 'Reset' button
        """
        self._widget.CameraPlugin.ResetButton.setChecked(QtCore.Qt.Checked)
        self._widget.CameraPlugin.ResetButton.clicked.emit()
        self.assertImage('testReset.png')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
