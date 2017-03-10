#!/usr/bin/env python
import sys
import unittest
from PyQt5 import QtWidgets
import vtk

from peacock.ExodusViewer.plugins.ContourPlugin import main
from peacock.utils import Testing


class TestContourPlugin(Testing.PeacockImageTestCase):
    """
    Testing for ContourControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FileControl widget.
        """

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.initialize([self._filename])
        self._widget.VariablePlugin.VariableList.setCurrentIndex(2)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onWindowRequiresUpdate()

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that enabled status
        self.assertTrue(self._widget.ContourPlugin.isEnabled())
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertFalse(self._widget.ContourPlugin.ContourLevels.isEnabled())

        # Enable contours
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)

        # Test visibility
        self.assertTrue(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertTrue(self._widget.ContourPlugin.ContourLevels.isEnabled())

        # Test that things initialize correctly
        self.assertImage('testInitial.png')

    def testContourCount(self):
        """
        Test that contour count is working.
        """
        # Enable contours
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)

        # Set count
        self._widget.ContourPlugin.ContourCount.setValue(6)
        self.assertImage('testContourCount.png')

    def testContourLevels(self):
        """
        Test that the contour levels is working.
        """
        # Enable contours
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)

        # Set levels
        self._widget.ContourPlugin.ContourLevels.setText('0.0001 0.5 1 2')
        self._widget.ContourPlugin.ContourLevels.editingFinished.emit()

        # Count should be disabled
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())

        # Test that image result
        self.assertImage('testContourLevels.png')

    def testStoreSettings(self):
        """
        Test that settings are saved when toggling between axis.
        """

        # Store an initial state of the GUI
        self._widget.ContourPlugin.onVariableChanged('state0')
        self._widget.ContourPlugin.setChecked(False)
        self._widget.ContourPlugin.clicked.emit(False)

        # Switch variables
        self._widget.ContourPlugin.onVariableChanged('state1')

        # Enable contours
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)
        self._widget.ContourPlugin.ContourCount.setValue(6)

        # Switch back to state0
        self._widget.ContourPlugin.onVariableChanged('state0')
        self.assertFalse(self._widget.ContourPlugin.isChecked())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)

        # Switch back to state1
        self._widget.ContourPlugin.onVariableChanged('state1')
        self.assertTrue(self._widget.ContourPlugin.isChecked())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 6)

    @unittest.skip('in progress')
    def testElemental(self):
        """
        Test that elemental variable disables the contour.
        """
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)

        self._widget.VariablePlugin.VariableList.setCurrentIndex(0)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(0)

        self.assertFalse(self._widget.ContourPlugin.isEnabled())
        self.assertImage('testElemental.png')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
