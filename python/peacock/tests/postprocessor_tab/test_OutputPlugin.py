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
from PyQt5 import QtWidgets

from peacock.PostprocessorViewer.plugins.OutputPlugin import main
import unittest

class TestOutputPlugin(unittest.TestCase):
    """
    Test class for FigureWidget.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI.
        """

        filename = '../input/vpp_*.csv'
        self._control, self._widget = main([filename])

    def testLiveTable(self):
        """
        Test that an empty plot with two projection options gets created.
        """

        # Test that window appears
        self._control.LiveTableButton.clicked.emit(True)
        self.assertTrue(self._control.LiveTable.isVisible())

        # Test that data exists
        self.assertTrue(self._control.LiveTable.currentWidget().item(1,1), "3")

        # Change time
        self._widget.currentWidget().MediaControlPlugin.BackwardButton.clicked.emit(True)
        self.assertTrue(self._control.LiveTable.currentWidget().item(1,1), "2")



if __name__ == '__main__':
    import unittest
    unittest.main(module=__name__, verbosity=2)
