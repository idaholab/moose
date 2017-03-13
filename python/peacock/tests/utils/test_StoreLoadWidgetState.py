#!/usr/bin/env python
import sys
import unittest
from PyQt5 import QtWidgets
from peacock.utils import WidgetUtils

class TestLoadStoreWidgetTools(unittest.TestCase):
    """
    Class for testing load/storeWidget functions in qtutils.py.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    @classmethod
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
        for letter in 'ABCDEF':
            self._box.addItem(letter)

        # Spin box widget
        self._spin = QtWidgets.QSpinBox()

        # Define and connect callbacks
        def callbackBox():
            WidgetUtils.loadWidget(self._spin, self._box.currentText(), 'box', debug=True)
        self._box.currentIndexChanged.connect(callbackBox)

        def callbackSpin():
            WidgetUtils.storeWidget(self._spin, self._box.currentText(), 'box', debug=True)
        self._spin.valueChanged.connect(callbackSpin)

        # Add widgets to layout
        self._layout.addWidget(self._box)
        self._layout.addWidget(self._spin)
        self._widget.show()

    def testLoadStore(self):
        """
        Test the load/store functions.
        """

        # Adjust the spin box
        self._spin.setValue(4)
        self._spin.valueChanged.emit(4)

        # Change the combo and re-adjust spin box
        self._box.setCurrentIndex(1)
        self._box.currentIndexChanged.emit(1)
        self._spin.setValue(7)
        self._spin.valueChanged.emit(7)

        # Change box back to 0 and test spin value
        self._box.setCurrentIndex(0)
        self._box.currentIndexChanged.emit(0)
        self.assertEqual(self._spin.value(), 4)

        # Change box back to 1 and test spin value
        self._box.setCurrentIndex(1)
        self._box.currentIndexChanged.emit(1)
        self.assertEqual(self._spin.value(), 7)




if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
