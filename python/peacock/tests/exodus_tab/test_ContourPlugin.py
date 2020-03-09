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
        self._widget.FilePlugin.onSetFilenames([self._filename])
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onWindowRequiresUpdate()

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint())

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
        self._widget.ContourPlugin.ContourCount.valueChanged.emit(6)
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

        # Test error
        self._widget.ContourPlugin.ContourLevels.setText('wrong')
        self._widget.ContourPlugin.ContourLevels.editingFinished.emit()
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.styleSheet(), u'color:#ff0000')
        self.assertImage('testInitial.png')

    def testState(self):
        """
        Test that settings are saved when toggling between axis.
        """

        # Change to diffused
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)

        # Check initial state
        self.assertFalse(self._widget.ContourPlugin.isChecked())
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertFalse(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '')

        # Enable contours
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)
        self.assertTrue(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertTrue(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '')

        # Change levels
        self._widget.ContourPlugin.ContourCount.setValue(6)
        self._widget.ContourPlugin.ContourCount.valueChanged.emit(6)
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 6)

        # Switch back to convected
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.ContourPlugin.isChecked())
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertFalse(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '')

        # Apply levels
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)
        self._widget.ContourPlugin.ContourLevels.setText('0.0001 0.5 1 2')
        self._widget.ContourPlugin.ContourLevels.editingFinished.emit()
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '0.0001 0.5 1 2')
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertTrue(self._widget.ContourPlugin.ContourLevels.isEnabled())

        # Switch back to diffused
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertTrue(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertTrue(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 6)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '')

        # Switch back to convected
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertTrue(self._widget.ContourPlugin.isChecked())
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertTrue(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '0.0001 0.5 1 2')

        # Switch to elemental
        self._widget.FilePlugin.VariableList.setCurrentIndex(0)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(0)
        self.assertFalse(self._widget.ContourPlugin.isChecked())
        self.assertFalse(self._widget.ContourPlugin.ContourCount.isEnabled())
        self.assertFalse(self._widget.ContourPlugin.ContourLevels.isEnabled())
        self.assertEqual(self._widget.ContourPlugin.ContourCount.value(), 10)
        self.assertEqual(self._widget.ContourPlugin.ContourLevels.text(), '')

    @unittest.skip('in progress')
    def testElemental(self):
        """
        Test that elemental variable disables the contour.
        """
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)

        self._widget.FilePlugin.VariableList.setCurrentIndex(0)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(0)

        self.assertFalse(self._widget.ContourPlugin.isEnabled())
        self.assertImage('testElemental.png')


    def testBlock(self):
        """
        Test that contour flag disables nodeset/sidesets.
        """
        self._widget.FilePlugin.VariableList.setCurrentIndex(0)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(0)

        self.assertTrue(self._widget.BlockPlugin.BlockSelector.isEnabled())
        self.assertFalse(self._widget.BlockPlugin.NodesetSelector.isEnabled())
        self.assertFalse(self._widget.BlockPlugin.SidesetSelector.isEnabled())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
