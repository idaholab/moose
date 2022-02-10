#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", 'peacock'))
import unittest
import peacock
from PyQt5 import QtWidgets, QtCore, QtGui
from UnitConverterPlugin import main

class TestUnitConverterPlugin(unittest.TestCase):
    """
    Tests for UnitConverterPlugin
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        self._widget = main(size = [600, 600])

    def testTextual(self):
        self._widget.SearchEdit.setText("32 F in C")
        self.assertAlmostEqual(float(self._widget.OutValue.text()), 0., places = 5)

    def testSelectType(self):
        for t in [ 'Length', 'Pressure', 'Temperature' ]:
            self._widget.UnitGroup.setCurrentText(t)
            self.assertTrue(self._widget.UnitGroup.currentText() == t)

    def testConversion(self):
        self._widget.UnitGroup.setCurrentText('Length')
        self._widget.InUnit.setCurrentText('Meter')
        self._widget.OutUnit.setCurrentText('Inch')

        # convert 'in' to 'out'
        self._widget.InValue.setText('1.')
        self.assertAlmostEqual(float(self._widget.OutValue.text()), 39.3701, places = 4)

        # convert 'out' to 'in'
        self._widget.OutValue.setText('72')
        self.assertAlmostEqual(float(self._widget.InValue.text()), 1.8288, places = 4)

        # change 'in' unit
        self._widget.InUnit.setCurrentText('Centimeter')
        self.assertAlmostEqual(float(self._widget.OutValue.text()), 0.72, places = 4)


if __name__ == '__main__':
    unittest.main(module = __name__, verbosity = 2)
