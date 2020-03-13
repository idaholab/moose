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
from PyQt5 import QtCore, QtWidgets

from peacock.PostprocessorViewer.plugins.LineSettingsWidget import main
from peacock.utils import Testing

class TestLineSettingsWidget(Testing.PeacockImageTestCase):
    """
    Test class for the LineSettingsWidget.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI containing the ArtistToggleWidget and the matplotlib figure axes.
        """

        # Create widgets
        self._widget, self._toggle, self._window = main('data')

        # Create plotting function
        def graph():
            settings = self._toggle.settings()
            ax = self._window.axes()[settings.pop('axis')]
            self._window.clear()
            if self._toggle.isValid():
                ax.plot([0,1,2,4], [0,1,4,16], **settings)
            self._window.draw()

        self._toggle.clicked.connect(graph)

    def click(self):
        """
        Enable the plot.
        """
        self._toggle.CheckBox.setCheckState(QtCore.Qt.Checked)
        self._toggle.CheckBox.clicked.emit()

    def testEmpty(self):
        """
        Test that an empty plot is possible.
        """

        # Test for empty plot
        self.assertImage('testEmpty.png')

        # Test that controls are disabled
        self.assertFalse(self._toggle.ColorButton.isEnabled(), "ColorButton should be disabled.")
        self.assertFalse(self._toggle.PlotAxis.isEnabled(), "PlotAxis should be disabled.")
        self.assertFalse(self._toggle.LineStyle.isEnabled(), "LineStyle should be disabled.")
        self.assertFalse(self._toggle.LineWidth.isEnabled(), "LineWidth should be disabled.")
        self.assertFalse(self._toggle.MarkerStyle.isEnabled(), "MarkerStyle should be disabled.")
        self.assertFalse(self._toggle.MarkerSize.isEnabled(), "MarkerSize should be disabled.")
        self.assertIn('rgb(0, 0, 255, 255)', str(self._toggle.ColorButton.styleSheet()))

    def testToggle(self):
        """
        Test that a line appears.
        """
        self.click()
        self.assertImage('testToggle.png')

    def testColor(self):
        """
        Test that that line color can be changed.

        Note: This doesn't use the QColorDialog (it is assumed that this is working correctly)
        """
        self.click()
        self._toggle._settings['color'] = [1,0,0]
        self._toggle.update()
        self.assertImage('testColor.png')

    def testLineStyle(self):
        """
        Test that line style toggle is working.
        """
        self.click()
        self._toggle.LineStyle.setCurrentIndex(3)
        self.assertImage('testLineStyle.png')

    def testLineWidth(self):
        """
        Test that line width toggle is working.
        """
        self.click()
        self._toggle.LineWidth.setValue(6)
        self.assertImage('testLineWidth.png')

    def testMarkerStyleSize(self):
        """
        Test that markers style toggle is working.
        """
        self.click()

        # Disable line
        self._toggle.LineStyle.setCurrentIndex(4)
        self.assertFalse(self._toggle.LineWidth.isEnabled(), "LineWidth should be disabled.")

        self._toggle.MarkerStyle.setCurrentIndex(2)
        self._toggle.MarkerSize.setValue(12)
        self.assertTrue(self._toggle.MarkerSize.isEnabled(), "MarkerSize should be enabled.")
        self.assertImage('testMarkerStyleSize.png')

    def testRepr(self):
        """
        Test that repr() method is working.
        """
        self.click()
        output, imports = self._toggle.repr()
        exact = "axes0.plot(x, y, label='data', linestyle='-', linewidth=1, color=[0, 0, 1], marker='', markersize=1)"
        self.assertEqual(output[1], exact)

if __name__ == '__main__':
    import unittest
    unittest.main(module=__name__, verbosity=2)
