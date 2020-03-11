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
from PyQt5 import QtWidgets, QtCore

from peacock.ExodusViewer.ExodusViewer import main
from peacock.utils import Testing

class TestExodusViewer2(Testing.PeacockImageTestCase):
    """
    A second set of tests for the ExodusViewer.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filename = Testing.get_chigger_input('diffusion_4.e')

    def setUp(self):
        self._widget, self._main_window = main(size=[400,400])
        self._widget.onSetFilenames([self._filename])
        self._window = self._widget.currentWidget().VTKWindowPlugin

    def testLocalRange(self):
        """Tests the local range toggle."""
        clip = self._widget.currentWidget().ClipPlugin
        cbar = self._widget.currentWidget().ColorbarPlugin

        clip.setChecked(QtCore.Qt.Checked)
        clip.clicked.emit(QtCore.Qt.Checked)
        clip.ClipSlider.setSliderPosition(35)
        clip.ClipSlider.sliderReleased.emit()
        Testing.process_events(1)

        self.assertTrue(cbar.RangeMinimum.text().startswith('0.635'))
        self.assertTrue(cbar.RangeMaximum.text(), '1')
        self.assertImage('testLocalRange.png')

        cbar.ColorBarRangeType.setChecked(QtCore.Qt.Unchecked)
        cbar.ColorBarRangeType.stateChanged.emit(QtCore.Qt.Unchecked)
        Testing.process_events(1)
        self.assertTrue(cbar.RangeMinimum.text(), '0')
        self.assertTrue(cbar.RangeMaximum.text(), '1')
        self.assertImage('testLocalRange2.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
