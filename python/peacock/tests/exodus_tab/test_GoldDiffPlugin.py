#!/usr/bin/env python
import os
import sys
import unittest
from PyQt5 import QtCore, QtWidgets
import vtk

from peacock.ExodusViewer.plugins.GoldDiffPlugin import main
from peacock.utils import Testing

class TestGoldDiff(Testing.PeacockImageTestCase):
    """
    Testing for GoldDiffPlugin widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin and GoldDiffPlugin widget.
        """

        self.sleepIfSlow()
        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e')
        self._widget, self._window = main(size=[400,400])
        self._widget.initialize(self._filenames)

    def write(self, filename, **kwargs):
        """
        Write the image file to be compared.

        NOTE: Requires self._window to be a VTKWindowWidget or FigureWidget object.
        """
        window = kwargs.pop('window', self._window)
        window.onWrite(filename, **kwargs)

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that things initialize correctly
        self.assertEqual(2, self._widget.FilePlugin.AvailableFiles.count())
        self.assertEqual(os.path.basename(self._filenames[0]), str(self._widget.FilePlugin.AvailableFiles.currentText()))
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testInitial.png')

    def testGold(self):
        """
        Test the gold toggle is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.isEnabled())
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)
        self.assertImage('testGold.png', window=self._widget.GoldDiffPlugin.GoldVTKWindow)

    def testDiff(self):
        """
        Test the gold toggle is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.isEnabled())
        self._widget.GoldDiffPlugin.DiffToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.DiffToggle.clicked.emit(True)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testDiff.png', window=self._widget.GoldDiffPlugin.DiffVTKWindow)

    def testCameraLink(self):
        """
        Test the gold toggle is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.LinkToggle.isEnabled())
        self._widget.GoldDiffPlugin.DiffToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.DiffToggle.clicked.emit(True)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)
        self._window.cameraChanged.emit(self._window._result.getVTKRenderer().GetActiveCamera())
        self.assertImage('testCameraLink.png', window=self._widget.GoldDiffPlugin.DiffVTKWindow)

        # Disconnect link, move the main window, the diff shouldn't move
        self._widget.GoldDiffPlugin.LinkToggle.setChecked(QtCore.Qt.Unchecked)
        self._widget.GoldDiffPlugin.LinkToggle.clicked.emit(False)
        camera.SetViewUp(0.7786, -0.2277, 0.5847)
        camera.SetPosition(9.2960, 0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)
        self._window.cameraChanged.emit(self._window._result.getVTKRenderer().GetActiveCamera())
        self.assertImage('testCameraLink.png', window=self._widget.GoldDiffPlugin.DiffVTKWindow)

    def testDisabled(self):
        """
        Test the gold/diff window gets disabled when a file doesn't have gold.
        """
        self._widget.FilePlugin.AvailableFiles.setCurrentIndex(1)
        self._widget.FilePlugin.AvailableFiles.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.GoldDiffPlugin.isEnabled())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
