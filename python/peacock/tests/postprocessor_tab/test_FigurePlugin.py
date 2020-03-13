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

from peacock.PostprocessorViewer.plugins.FigurePlugin import main
from peacock.utils import Testing
import mooseutils

class TestFigurePlugin(Testing.PeacockImageTestCase):
    """
    Test class for FigureWidget.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI.
        """

        # Read some data
        filename = '../input/white_elephant_jan_2016.csv'
        self._reader = mooseutils.PostprocessorReader(filename)

        # Create the widget with FigurePlugin only
        self._widget = main()
        self._window = self._widget.currentWidget().FigurePlugin

    def testEmpty(self):
        """
        Test that an empty plot with two projection options gets created.
        """
        self._window.draw()
        self.assertImage('testEmpty.png')

    def testPlotLeft(self):
        """
        Draws on left axis.
        """
        ax = self._window.axes()[0]
        ax.plot(self._reader['air_temp_low_24_hour_set_1'], '-b')
        self._window.draw()
        self.assertImage('testPlotLeft.png')

    def testPlotRight(self):
        """
        Draws right axis.
        """
        ax = self._window.axes()[1]
        ax.plot(self._reader['air_temp_high_24_hour_set_1'], '-r')
        self._window.draw()
        self.assertImage('testPlotRight.png')

    def testPlotDual(self):
        """
        Draws on both.
        """
        ax = self._window.axes()[0]
        ax.plot(self._reader['air_temp_low_24_hour_set_1'], '-b')

        ax = self._window.axes()[1]
        ax.plot(self._reader['air_temp_high_24_hour_set_1'], '-r')

        self._window.draw()
        self.assertImage('testPlotDual.png')

    def testClear(self):
        """
        Test that a plot can be created and cleared.
        """

        ax = self._window.axes()[0]
        ax.plot(self._reader['snow_water_equiv_set_1'], '-b')
        self._window.draw()
        self.assertImage('testClearPlot.png')

        ax.clear()
        self._window.draw()
        self.assertImage('testEmpty.png')

    def testRepr(self):
        """
        Test the "repr" script output.
        """

        output, imports = self._window.repr()
        self.assertIn('import matplotlib.pyplot as plt', imports)
        self.assertIn("figure = plt.figure(facecolor='white')", output)
        self.assertIn('axes0 = figure.add_subplot(111)', output)

        # This only appears if data exists on axes2
        ax1 = 'axes1 = axes0.twinx()'
        self.assertNotIn(ax1, output)

        # Plot data on right and make sure axes1 appears
        ax = self._window.axes()[1]
        ax.plot(self._reader['air_temp_high_24_hour_set_1'], '-r')
        output, imports = self._window.repr()
        self.assertIn(ax1, output)

if __name__ == '__main__':
    import unittest
    unittest.main(module=__name__, verbosity=2)
