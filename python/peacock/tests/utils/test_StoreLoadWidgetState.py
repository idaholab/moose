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
from peacock.utils import WidgetUtils

class TestLoadStoreWidgetTools(unittest.TestCase):
    """
    Class for testing load/storeWidget functions in qtutils.py.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates widget to test load/store functionality.
        """

        # The main widget
        self._widget = QtWidgets.QWidget()
        self._layout = QtWidgets.QVBoxLayout()
        self._widget.setLayout(self._layout)

        # Box widget
        self._box = QtWidgets.QComboBox()
        for letter in 'BCDEF':
            self._box.addItem(letter)

        # Spin box widget
        self._spin = QtWidgets.QSpinBox()

        # Define and connect callbacks
        def callbackBox():
            WidgetUtils.storeWidget(self._box, self._spin.value(), debug=True)
        self._box.currentIndexChanged.connect(callbackBox)

        def callbackSpin():
            WidgetUtils.loadWidget(self._box, self._spin.value(), debug=True)
        self._spin.valueChanged.connect(callbackSpin)

        # Add widgets to layout
        self._layout.addWidget(self._box)
        self._layout.addWidget(self._spin)
        self._widget.show()

    def testLoadStore(self):
        """
        Test the load/store functions.
        """
        # Adjust ComboBox (Stores state of ComboBox with spin value 0)
        self._box.setCurrentIndex(4)
        self.assertEqual(self._box.currentText(), 'F')

        # Change the SpinBox
        self._spin.setValue(7)
        self.assertEqual(self._spin.value(), 7)

        # Adjust ComboBox (Stores state of ComboBox with spin value 7)
        self._box.setCurrentIndex(1)
        self.assertEqual(self._box.currentText(), 'C')

        # Change SpinBox to 0 (Loads state of ComboBox with spin value 0)
        self._spin.setValue(0)
        self.assertEqual(self._spin.value(), 0)
        self.assertEqual(self._box.currentText(), 'F')

        # Change SpinBox to 7 (Loads state of ComboBox with spin value 7)
        self._spin.setValue(7)
        self.assertEqual(self._spin.value(), 7)
        self.assertEqual(self._box.currentText(), 'C')

        # Add a value to QComboBox
        self._box.blockSignals(True)
        self._box.clear()
        for letter in 'ABCDEF':
            self._box.addItem(letter)
        self._box.blockSignals(False)
        WidgetUtils.loadWidget(self._box, self._spin.value())
        self.assertEqual(self._box.currentText(), 'C')

        self._spin.setValue(0)
        self.assertEqual(self._box.currentText(), 'F')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
