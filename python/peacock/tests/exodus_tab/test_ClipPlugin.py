#!/usr/bin/env python
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

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._window.onFileChanged(self._filename)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that enabled status
        self.assertFalse(self._widget.ClipPlugin.ClipDirection.isEnabled())
        self.assertFalse(self._widget.ClipPlugin.ClipSlider.isEnabled())

        # Enable clipping
        self._widget.ClipPlugin.ClipToggle.setChecked(True)

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
        self._widget.ClipPlugin.ClipToggle.setChecked(True)

        # y-axis
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self.assertImage('testSelectAxisY.png')

        # z-axis
        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(2)
        self.assertImage('testSelectAxisZ.png')

    def testSlider(self):
        """
        Test the origin slider.
        """

        # Enable clipping
        self._widget.ClipPlugin.ClipToggle.setChecked(True)

        # Update the slider
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(15)
        self._widget.ClipPlugin.ClipSlider.sliderReleased.emit()
        self.assertImage('testSlide.png')

    def testStoreSettings(self):
        """
        Test that slider position settings are saved when toggling between axis.
        """

        # Enable clipping
        self._widget.ClipPlugin.ClipToggle.setChecked(True)

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

    def testVariableStoreSettings(self):
        """
        Test that settings are saved when toggling between axis.
        """

        # Store an initial state of the GUI
        self._widget.ClipPlugin.onVariableChanged('state0')
        self._widget.ClipPlugin.ClipToggle.setChecked(False)

        # Switch variables
        self._widget.ClipPlugin.onVariableChanged('state1')

        # Enable contours
        self._widget.ClipPlugin.ClipToggle.setChecked(True)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self._widget.ClipPlugin.ClipSlider.setSliderPosition(15)
        self._widget.ClipPlugin.ClipSlider.sliderReleased.emit()

        # Switch back to state0
        self._widget.ClipPlugin.onVariableChanged('state0')
        self.assertFalse(self._widget.ClipPlugin.ClipToggle.isChecked())
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 20)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentIndex(), 0)

        # Switch back to state1
        self._widget.ClipPlugin.onVariableChanged('state1')
        self.assertTrue(self._widget.ClipPlugin.ClipToggle.isChecked())
        self.assertEqual(self._widget.ClipPlugin.ClipSlider.sliderPosition(), 15)
        self.assertEqual(self._widget.ClipPlugin.ClipDirection.currentIndex(), 1)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
