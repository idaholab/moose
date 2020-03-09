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
from peacock.PostprocessorViewer.plugins.AxisTabsPlugin import main
from peacock.utils import Testing

class TestAxisTabsPlugin(Testing.PeacockImageTestCase):
    """
    Test class for Axis settings.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI.
        """
        self._control, self._widget, self._window = main(['../input/white_elephant_jan_2016.csv'])
        self._tabs = [self._control.XAxisTab, self._control.YAxisTab, self._control.Y2AxisTab]

    def plot(self):

        # Create plot
        select = self._widget.currentWidget().PostprocessorSelectPlugin
        var = 'air_temp_set_1'
        select._groups[0]._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        select._groups[0]._toggles[var].CheckBox.clicked.emit(True)

        var = 'snow_depth_set_1'
        select._groups[0]._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        select._groups[0]._toggles[var].PlotAxis.setCurrentIndex(1)
        select._groups[0]._toggles[var].CheckBox.clicked.emit(True)

    def testEmpty(self):
        """
        Test empty plot.
        """

        for tab in self._tabs:
            self.assertEqual(tab.Label.text(), '')
            self.assertEqual(tab.RangeMinimum.text(), '')
            self.assertEqual(tab.RangeMaximum.text(), '')
            self.assertFalse(tab.GridToggle.isChecked())
            self.assertFalse(tab.Scale.isChecked())

    def testAuto(self):
        """
        Test the auto range limits show up when plot is created.
        """
        self.plot()
        ranges = [[-1.548, 32.508], [4.26, 35.94], [40.6, 181.4]]
        for i in range(3):
            self.assertEqual(float(self._tabs[i].RangeMinimum.text()), ranges[i][0])
            self.assertEqual(float(self._tabs[i].RangeMaximum.text()), ranges[i][1])

    def testCustom(self):
        """
        Test that the limits and labels can be customized.
        """
        self.plot()
        labels = ['xxxxxxxxxx', 'yyyyyyyyy', 'y2y2y2y2y2y2y2y2y2']
        ranges = [[5.0, 25.0], [10.0, 25.2], [1, 100]]
        for i in range(3):
            self._tabs[i].Label.setText(labels[i])
            self._tabs[i].Label.editingFinished.emit()
            self._tabs[i].RangeMinimum.setText(str(ranges[i][0]))
            self._tabs[i].RangeMinimum.editingFinished.emit()
            self._tabs[i].RangeMaximum.setText(str(ranges[i][1]))
            self._tabs[i].RangeMaximum.editingFinished.emit()

        self.assertEqual(self._control.axes(0).get_xlabel(), labels[0])
        self.assertEqual(self._control.axes(0).get_ylabel(), labels[1])
        self.assertEqual(self._control.axes(1).get_ylabel(), labels[2])

        self.assertEqual(list(self._control.axes(0).get_xlim()), ranges[0])
        self.assertEqual(list(self._control.axes(0).get_ylim()), ranges[1])
        self.assertEqual(list(self._control.axes(1).get_ylim()), ranges[2])

        self.assertImage('testCustom.png')

    def testScale(self):
        """
        Test that scaling is working.
        """

        self._window.clear()
        x = [0,    1,   2, 3,  4,   5]
        y = [0.01, 0.1, 1, 10, 100, 1000]
        self._window.axes(0).plot(x, y, '-k')

        self._tabs[1].Scale.setCheckState(QtCore.Qt.Checked)
        self._tabs[1].Scale.clicked.emit(True)
        self.assertImage('testScale.png')

    def testGrid(self):
        """
        Test that grid lines work.
        """
        self.plot()
        for i in range(2):
            self._tabs[i].GridToggle.setCheckState(QtCore.Qt.Checked)
            self._tabs[i].GridToggle.clicked.emit(True)
        self.assertImage('testGrid.png')

    def testRepr(self):
        """
        Test python scripting.
        """
        self.plot()
        output, imports = self._control.repr()
        self.assertIn("axes0.set_xlabel('time')", output)
        self.assertIn("axes0.set_ylabel('air_temp_set_1')", output)
        self.assertIn("axes1.set_ylabel('snow_depth_set_1')", output)
        self.assertIn("axes0.set_xlim([-1.548, 32.508])", output)
        self.assertIn("axes0.set_ylim([4.26, 35.94])", output)
        self.assertIn("axes1.set_ylim([40.6, 181.4])", output)
        self.assertNotIn("axes0.grid(True, axis='x')", output)
        self.assertNotIn("axes0.grid(True, axis='y')", output)
        self.assertNotIn("axes0.set_yscale('log')", output)

        # Enable grid/scale
        self._tabs[1].Scale.setCheckState(QtCore.Qt.Checked)
        self._tabs[1].Scale.clicked.emit(True)
        for i in range(2):
            self._tabs[i].GridToggle.setCheckState(QtCore.Qt.Checked)
            self._tabs[i].GridToggle.clicked.emit(True)
        output, imports = self._control.repr()
        self.assertIn("axes0.grid(True, axis='x')", output)
        self.assertIn("axes0.grid(True, axis='y')", output)
        self.assertNotIn("axes1.grid(True, axis='y')", output)
        self.assertIn("axes0.set_yscale('log')", output)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
