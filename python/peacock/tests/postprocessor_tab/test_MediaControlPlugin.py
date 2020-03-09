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
from PyQt5 import QtCore, QtWidgets
from peacock.PostprocessorViewer.plugins.MediaControlPlugin import main
from peacock.utils import Testing


class TestMediaControlPlugin(Testing.PeacockImageTestCase):
    """
    Test class for the MediaControls with Postprocessor data.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Load necessary plugins for testing.
        """
        filenames = ['../input/vpp_*.csv', '../input/vpp2_*.csv']
        self._widget = main(filenames)

        self._select = self._widget.currentWidget().PostprocessorSelectPlugin
        self._control = self._widget.currentWidget().MediaControlPlugin
        self._window = self._widget.currentWidget().FigurePlugin
        self._window.setFixedSize(QtCore.QSize(400, 400))

    def plot(self):
        """
        Plot two lines, one from each data set.
        """
        vars = ['y', 't*x**2']
        for i in range(len(vars)):
            self._select._groups[i]._toggles[vars[i]].CheckBox.setCheckState(QtCore.Qt.Checked)
            if i == 1:
                self._select._groups[i]._toggles[vars[i]].PlotAxis.setCurrentIndex(1)
            self._select._groups[i]._toggles[vars[i]].CheckBox.clicked.emit(True)

    def testInitial(self):
        """
        Test that an empty plot is possible.
        """
        self.assertImage('testInitial.png')

    def testInitialize(self):
        """
        Test that controls are initialized.
        """
        self.plot()

        # Test the proper time/slider information exists
        self.assertEqual(int(self._control.TimeStepDisplay.text()), 5)
        self.assertEqual(float(self._control.TimeDisplay.text()), 9)
        self.assertEqual(int(self._control.TimeSlider.sliderPosition()), 5)

        # Test that proper items are disabled
        self.assertFalse(self._control.EndButton.isEnabled())
        self.assertFalse(self._control.ForwardButton.isEnabled())
        self.assertTrue(self._control.BeginButton.isEnabled())
        self.assertTrue(self._control.BackwardButton.isEnabled())

        self.assertImage('testInitialize.png')

    def testBeginButton(self):
        """
        Test that beginning button is working
        """
        self.plot()

        # Press the begin button an test result image
        self._control.BeginButton.clicked.emit(True)
        self.assertImage('testBeginButton.png')

        # Test that proper items are disabled
        self.assertTrue(self._control.EndButton.isEnabled())
        self.assertTrue(self._control.ForwardButton.isEnabled())
        self.assertFalse(self._control.BeginButton.isEnabled())
        self.assertFalse(self._control.BackwardButton.isEnabled())

    def testEndButton(self):
        """
        Test that the end button is working.
        """
        self.plot()
        self._control.BeginButton.clicked.emit(True)
        self._control.EndButton.clicked.emit(True)
        self.assertImage('testInitialize.png')

    def testBackwardButton(self):
        """
        Test that pressing the backward button does something.
        """
        self.plot()

        self._control.BackwardButton.clicked.emit(True)
        self.assertEqual(int(self._control.TimeStepDisplay.text()), 4)
        self.assertEqual(float(self._control.TimeDisplay.text()), 7)
        self.assertImage('testBackwardButton_A.png')

        self._control.BackwardButton.clicked.emit(True)
        self.assertEqual(int(self._control.TimeStepDisplay.text()), 3)
        self.assertEqual(float(self._control.TimeDisplay.text()), 5)
        self.assertImage('testBackwardButton_B.png')

    def testForwardButton(self):
        """
        Test that pressing the forward button does something.
        """
        self.plot()
        self._control.BackwardButton.clicked.emit(True)
        self._control.BackwardButton.clicked.emit(True)
        self._control.ForwardButton.clicked.emit(True)

        self.assertEqual(int(self._control.TimeStepDisplay.text()), 4)
        self.assertEqual(float(self._control.TimeDisplay.text()), 7)
        self.assertImage('testBackwardButton_A.png')

    def testTimeStepEdit(self):
        """
        Test that moving to a timestep by typing.
        """
        self.plot()
        self._control.TimeStepDisplay.setText('1.2')
        self._control.TimeStepDisplay.editingFinished.emit()
        self.assertEqual(float(self._control.TimeDisplay.text()), 1)
        self.assertImage('testTimeStepEdit.png')

    def testTimeEdit(self):
        """
        Test that moving to a time by typing.
        """
        self.plot()
        self._control.TimeDisplay.setText('7.2') # should take you to step 4
        self._control.TimeDisplay.editingFinished.emit()
        self.assertEqual(int(self._control.TimeStepDisplay.text()), 4)
        self.assertAlmostEqual(float(self._control.TimeDisplay.text()), 7)
        self.assertImage('testTimeEdit.png')

    def testRepr(self):
        """
        Test python scripting.
        """
        self.plot()
        output, imports = self._control.repr() # This plugin doesn't contribute to the script
        self.assertEqual(output, [])
        self.assertEqual(imports, [])

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
