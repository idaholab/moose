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

from peacock.ExodusViewer.plugins.ClipPlugin import main
from peacock.utils import Testing

class TestClipPlugin(Testing.PeacockImageTestCase):
    """
    Testing for FileControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FileControl widget.
        """

        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onWindowRequiresUpdate()

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that enabled status
        self.assertFalse(self._widget.ClipPlugin.ClipDirection.isEnabled())
        self.assertFalse(self._widget.ClipPlugin.ClipSlider.isEnabled())

        # Enable clipping
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)

        # Test visibility
        self.assertTrue(self._widget.ClipPlugin.ClipDirection.isEnabled())
        self.assertTrue(self._widget.ClipPlugin.ClipSlider.isEnabled())

        # Test that things initialize correctly
        self.assertImage('testInitial.png')

    def testSelectAxis(self):
        """
        Test that the axis selection is operating.
        """

        # Enable clipping
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)

        # y-axis
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self.assertImage('testSelectAxisY.png')

        # z-axis
        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint())
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(2)
        self.assertImage('testSelectAxisZ.png')

    def testSlider(self):
        """
        Test the origin slider.
        """

        # Enable clipping
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)

        # Update the slider
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(15)
        self._widget.ClipPlugin.ClipSlider.sliderReleased.emit()
        self.assertImage('testSlide.png')

    def testSliderState(self):
        """
        Test that slider position settings are saved when toggling between axis.
        """

        # Enable clipping
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)

        # Test the initial slider position
        self.assertEqual(20, self._widget.ClipPlugin.ClipSlider.sliderPosition())

        # Change axis and slider position
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(15)
        self._widget.ClipPlugin.ClipSlider.sliderReleased.emit()

        # Change direction back and check slider position
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(0)
        self.assertEqual(20, self._widget.ClipPlugin.ClipSlider.sliderPosition())

        # Switch back to y-axis and check slider position
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self.assertEqual(15, self._widget.ClipPlugin.ClipSlider.sliderPosition())

        # Test that the y-axis cut looks correct
        self.assertImage('testStoreSettings.png')

    def testState(self):
        """
        Test that settings are saved when toggling between axis.
        """
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)

        # Check initial state
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "X")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 20)

        # Update slider
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(5)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Y")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 5)

        # Change variable
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.ClipPlugin.isChecked())

        # Change back
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertTrue(self._widget.ClipPlugin.isChecked())
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Y")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 5)

        # Change back to again
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "X")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 20)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(2)
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(30)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Z")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 30)

        # Change variable back
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Y")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 5)

        # Change again
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Z")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 30)

    def testState2(self):
        """
        Test that settings are saved across files.
        """
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(5)
        self.assertTrue(self._widget.ClipPlugin.isChecked())
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Y")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 5)

        # Change files
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.ClipPlugin.isChecked())

        # Change back
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertTrue(self._widget.ClipPlugin.isChecked())
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentText(), "Y")
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 5)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
