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
from peacock.ExodusViewer.plugins.MediaControlPlugin import main
from peacock.utils import Testing

class TestMediaControlPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to BlockControls widget.
        """

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._window.onSetFilename(self._filename)
        self._window.onSetVariable('diffused')
        self._window.onWindowRequiresUpdate()
        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))

    def testInitial(self):
        """
        Test the initial result.
        """
        self.assertTrue(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.PlayButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.PauseButton.isVisible())
        self.assertFalse(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.EndButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.TimeStepDisplay.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.TimeDisplay.isEnabled())
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '20')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '2.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 20)
        self.assertImage('testInitial.png')

    def testBeginButton(self):
        """
        Test that the begin button works.
        """
        self._widget.MediaControlPlugin.BeginButton.clicked.emit(True)

        self.assertFalse(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.EndButton.isEnabled())

        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 0)

        self.assertImage('testBeginButton.png')

    def testBackwardButton(self):
        """
        Test that the back button works.
        """

        # Click button 10 times
        for i in range(10):
            self._widget.MediaControlPlugin.BackwardButton.clicked.emit(True)

        self.assertTrue(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.EndButton.isEnabled())

        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '10')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '1.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 10)

        self.assertImage('testBackwardButton.png')

    def testPlayButton(self):
        """
        Test that play button works.
        """

        # Go to timestep 10
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('10')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()

        # Test the pressing the play button the pause button appears and the other controls are disabled
        self._widget.MediaControlPlugin.PlayButton.clicked.emit(True)
        self.assertTrue(self._widget.MediaControlPlugin.PauseButton.isVisible())
        self.assertFalse(self._widget.MediaControlPlugin.PlayButton.isVisible())
        self.assertFalse(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.EndButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.TimeStepDisplay.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.TimeDisplay.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.TimeSlider.isEnabled())

        # The play button uses a timer, which doesn't work in the testing framework, so fake it
        for i in range(10):
            self._widget.MediaControlPlugin.Timer.timeout.emit()
        self.assertImage('testPlayButton.png')

    def testForwardButton(self):
        """
        Test that forward button is working.
        """

        # Go to the beginning and advance 5 steps
        self._widget.MediaControlPlugin.BeginButton.clicked.emit(True)
        for i in range(5):
            self._widget.MediaControlPlugin.ForwardButton.clicked.emit(True)

        self.assertTrue(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.EndButton.isEnabled())

        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '5')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.5')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 5)

        self.assertImage('testForwardButton.png')

    def testEndButton(self):
        """
        Test that end button is working.
        """

        # Go to the beginning and advance 5 steps
        self._widget.MediaControlPlugin.BeginButton.clicked.emit(True)
        self._widget.MediaControlPlugin.EndButton.clicked.emit(True)

        self.assertTrue(self._widget.MediaControlPlugin.BeginButton.isEnabled())
        self.assertTrue(self._widget.MediaControlPlugin.BackwardButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.ForwardButton.isEnabled())
        self.assertFalse(self._widget.MediaControlPlugin.EndButton.isEnabled())

        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '20')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '2.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 20)

        self.assertImage('testInitial.png')

    def testTimeStepEdit(self):
        """
        Test that timestep can be set manually.
        """

        # Test in-bounds value (although not an exact value)
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('12.1')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '12')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '1.2')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 12)
        self.assertImage('testTimeStepEdit.png')

        # Test -1 timestep
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('-1')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '20')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '2.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 20)

        # Test over range timestep
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('500')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '20')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '2.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 20)

        # Test under range timestep
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('-999')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 0)

    def testTimeEdit(self):
        """
        Test the time edit box.
        """
        # Test in-bounds value (although not an exact value)
        self._widget.MediaControlPlugin.TimeDisplay.setText('0.619')
        self._widget.MediaControlPlugin.TimeDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '6')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.6')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 6)
        self.assertImage('testTimeEdit.png')

        # Test over range timestep
        self._widget.MediaControlPlugin.TimeDisplay.setText('500')
        self._widget.MediaControlPlugin.TimeDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '20')
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '2.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 20)

        # Test under range timestep
        self._widget.MediaControlPlugin.TimeDisplay.setText('-999')
        self._widget.MediaControlPlugin.TimeDisplay.editingFinished.emit()
        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.0')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 0)

    def testTimeSlider(self):
        """
        Test that time slider is working.
        """
        self._widget.MediaControlPlugin.TimeSlider.setValue(4)
        self._widget.MediaControlPlugin.TimeSlider.sliderReleased.emit()

        self.assertEqual(self._widget.MediaControlPlugin.TimeStepDisplay.text(), '4')
        self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.4')
        self.assertEqual(self._widget.MediaControlPlugin.TimeSlider.value(), 4)
        self.assertImage('testTimeSlider.png')

    def testMeshOnly(self):
        """
        Test that mesh only disables media controls.
        """
        filename = Testing.get_chigger_input('mesh_only.e')
        self._window.onSetFilename(filename)
        self._window.onWindowRequiresUpdate()
        #self.assertFalse(self._widget.MediaControlPlugin.isEnabled())
        self.assertImage('testMeshOnly.png')

    def testPlayDisable(self):
        """
        Tests that play button disables other plugins.
        """

        # Set the time so there is something to play
        self._widget.MediaControlPlugin.TimeStepDisplay.setText('10')
        self._widget.MediaControlPlugin.TimeStepDisplay.editingFinished.emit()

        # Other plugins should still be enabled
        self.assertTrue(self._widget.ClipPlugin.isEnabled())

        # Test the pressing the play button the pause button appears and the other plugins are disabled
        self._widget.MediaControlPlugin.PlayButton.clicked.emit(True)
        self.assertFalse(self._widget.ClipPlugin.isEnabled())

        # The play button uses a timer, which doesn't work in the testing framework, so fake it
        for i in range(10):
            self.assertFalse(self._widget.ClipPlugin.isEnabled())
            self._widget.MediaControlPlugin.Timer.timeout.emit()
        self.assertImage('testPlayButton.png')

        # Test that stop re-enables
        self._widget.MediaControlPlugin.stop()
        self.assertTrue(self._widget.ClipPlugin.isEnabled())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
