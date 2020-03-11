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

from peacock.PostprocessorViewer.plugins.AxesSettingsPlugin import main
from peacock.utils import Testing


class TestAxesSettingsPlugin(Testing.PeacockImageTestCase):
    """
    Test class for the ArtistToggleWidget which toggles postprocessor lines.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI.
        """
        self._control, self._widget, self._window = main(['../input/white_elephant_jan_2016.csv'])

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
        Test that an empty plot is possible.
        """
        ax0, ax1 = self._window.axes()
        self.assertEqual(ax0.get_xlabel(), '')
        self.assertEqual(ax0.get_ylabel(), '')
        self.assertEqual(ax1.get_ylabel(), '')

        self.assertFalse(self._control.LegendLocation.isEnabled())
        self.assertFalse(self._control.Legend2Location.isEnabled())
        self.assertImage('testEmpty.png')

    def testTitle(self):
        """
        Test that a title may be added.
        """
        self._control.Title.setText("Testing Title")
        self._control.Title.editingFinished.emit()
        ax0, ax1 = self._window.axes()
        self.assertEqual(ax0.get_title(), "Testing Title")

    def testLegend(self):
        """
        Test that legend toggle operates.
        """
        self.plot()
        self._control.Legend.setCheckState(QtCore.Qt.Checked)
        self._control.Legend.clicked.emit(True)
        self._control.Legend2.setCheckState(QtCore.Qt.Checked)
        self._control.Legend2.clicked.emit(True)
        self.assertTrue(self._control.LegendLocation.isEnabled())
        self.assertTrue(self._control.Legend2Location.isEnabled())
        self.assertImage('testLegend.png')

    def testLegendLocation(self):
        """
        Test legend location selection.
        """
        self.plot()
        self._control.Legend.setCheckState(QtCore.Qt.Checked)
        self._control.Legend.clicked.emit(True)
        self._control.LegendLocation.setCurrentIndex(3)
        self._control.Legend2.setCheckState(QtCore.Qt.Checked)
        self._control.Legend2.clicked.emit(True)
        self._control.Legend2Location.setCurrentIndex(1)
        self.assertImage('testLegendLocation.png')

    def testEmptyLegend(self):
        """
        Test that a legend created with no data produces warning.
        """

        # Enable legends
        self.plot()
        self._control.Legend.setCheckState(QtCore.Qt.Checked)
        self._control.Legend.clicked.emit(True)
        self._control.Legend2.setCheckState(QtCore.Qt.Checked)
        self._control.Legend2.clicked.emit(True)

        # Remove lines
        select = self._widget.currentWidget().PostprocessorSelectPlugin
        var = 'air_temp_set_1'
        select._groups[0]._toggles[var].CheckBox.setCheckState(QtCore.Qt.Unchecked)
        select._groups[0]._toggles[var].CheckBox.clicked.emit(False)

        var = 'snow_depth_set_1'
        select._groups[0]._toggles[var].CheckBox.setCheckState(QtCore.Qt.Unchecked)
        select._groups[0]._toggles[var].PlotAxis.setCurrentIndex(1)
        select._groups[0]._toggles[var].CheckBox.clicked.emit(False)
        self.assertImage('testEmpty.png')

    def testRepr(self):
        """
        Test python scripting.
        """
        self.plot()

        output, imports = self._control.repr()
        self.assertNotIn("axes0.legend(loc='best')", output)
        self.assertNotIn("axes1.legend(loc='best')", output)
        self.assertNotIn("axes0.set_title('Title')", output)

        self._control.Title.setText("Title")
        self._control.Legend.setCheckState(QtCore.Qt.Checked)
        self._control.Legend.clicked.emit(True)
        self._control.Legend2.setCheckState(QtCore.Qt.Checked)
        self._control.Legend2.clicked.emit(True)
        output, imports = self._control.repr()
        self.assertIn("axes0.legend(loc='best')", output)
        self.assertIn("axes1.legend(loc='best')", output)
        self.assertIn("axes0.set_title('Title')", output)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
