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
import os
import unittest
import shutil
from PyQt5 import QtCore, QtWidgets

from peacock.PostprocessorViewer.PostprocessorDataWidget import PostprocessorDataWidget
from peacock.PostprocessorViewer.plugins.LineGroupWidget import main
from peacock.utils import Testing
import mooseutils


class TestLineGroupWidgetVectorPostprocessor(Testing.PeacockImageTestCase):
    """
    Test class for the ArtistToggleWidget which toggles postprocessor lines.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)


    def copyfiles(self):
        """
        Copy the data file to a local temporary.
        """
        prefix = os.path.abspath(os.path.join(__file__, '../../input/vpp2_'))
        t = [0, 1, 3, 5, 7, 9]
        for i in range(len(t)):
            src = '{}000{}.csv'.format(prefix, t[i])
            shutil.copyfile(src, self._filenames[i])

    def create(self):
        """
        Creates the widgets for testing.

        This is done here rather than in setUp to allow for testing of delayed loading.
        """

        self._reader = mooseutils.VectorPostprocessorReader(self._pattern)
        self._data = PostprocessorDataWidget(self._reader)

        # Build the widgets
        self._control, self._widget, self._window = main(self._data)
        self._widget.currentWidget().FigurePlugin.setFixedSize(QtCore.QSize(625, 625))

    def setUp(self):
        """
        Creates the GUI containing the ArtistGroupWidget and the matplotlib figure axes.
        """

        prefix = self.__class__.__name__

        self._pattern = '{}_test_*.csv'.format(prefix)

        self._filenames = []
        for t in [0, 1, 3, 5, 7, 9]:
            self._filenames.append('{}_test_000{}.csv'.format(prefix, t))

    def tearDown(self):
        """
        Clean up.
        """
        for filename in self._filenames:
            if os.path.exists(filename):
                os.remove(filename)

    def testEmpty(self):
        """
        Test that an empty plot is possible.
        """
        self.copyfiles()
        self.create()

        self.assertImage('testEmpty.png')

        # Test that controls are initialized and disabled correctly
        self.assertEqual(self._control.AxisVariable.currentText(), "index (Peacock)")
        self.assertFalse(self._control._toggles['index (Peacock)'].isEnabled(), "id toggle should be disabled.")

    def testSelect(self):
        """
        Test that selecting variables works.
        """
        self.copyfiles()
        self.create()

        vars = ['t**2*x', 't*x**2']
        for var in vars:
            self._control._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
            self._control._toggles[var].clicked.emit()
        self.assertImage('testSelect.png')

        self.assertEqual('; '.join(vars), self._window.axes()[0].get_yaxis().get_label().get_text())
        self.assertEqual('index (Peacock)', self._window.axes()[0].get_xaxis().get_label().get_text())

        # Switch axis
        self._control._toggles[vars[0]].PlotAxis.setCurrentIndex(1)
        self._control._toggles[vars[0]].clicked.emit()
        self.assertImage('testSelect2.png')

        self.assertEqual(vars[0], self._window.axes()[1].get_yaxis().get_label().get_text())
        self.assertEqual(vars[1], self._window.axes()[0].get_yaxis().get_label().get_text())
        self.assertEqual('index (Peacock)', self._window.axes()[0].get_xaxis().get_label().get_text())

    def testChangePrimaryVariable(self):
        """
        Test that the primary variable may be modified.
        """

        self.copyfiles()
        self.create()

        # Plot something
        x_var = 't*x'
        y_var = 't*x**2'

        self._control._toggles[y_var].CheckBox.setCheckState(QtCore.Qt.Checked)
        self._control._toggles[y_var].clicked.emit()
        self.assertImage('testChangePrimaryVariable0.png')

        # Change the primary variable
        self._control.AxisVariable.setCurrentIndex(2)
        self._control.AxisVariable.currentIndexChanged.emit(2)
        self.assertEqual(self._control.AxisVariable.currentText(), x_var)
        self.assertFalse(self._control._toggles[x_var].isEnabled(), "Toggle should be disabled.")
        self.assertTrue(self._control._toggles['index (Peacock)'].isEnabled(), "Toggle should be enabled.")
        self.assertImage('testChangePrimaryVariable1.png')

    def testDelayLoadAndUnload(self):
        """
        Test that delayed loading and removal of files works.
        """
        self.create()

        # Plot should be empty and the message should be visible.
        self.assertImage('testEmpty.png')
        self.assertTrue(self._control.NoDataMessage.isVisible())

        # Load data
        self.copyfiles()
        self._data.load()
        self.assertFalse(self._control.NoDataMessage.isVisible())

        # Plot something
        var = 't*x**2'
        self._control._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        self._control._toggles[var].clicked.emit()
        self.assertImage('testDelayLoadPlot.png')

        # Remove data
        self.tearDown()
        self._data.load()
        self.assertTrue(self._control.NoDataMessage.isVisible())
        self.assertImage('testEmpty.png')

        # Re-load data
        self.copyfiles()
        self._data.load()
        self.assertFalse(self._control.NoDataMessage.isVisible())
        self._control._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        self._control._toggles[var].clicked.emit()
        self.assertImage('testDelayLoadPlot2.png') # The line color/style is different because the cycle keeps going

    def testTime(self):
        """
        Test that the time of VPP data can be specified.
        """
        self.copyfiles()
        self.create()
        var = 't*x**2'
        self._control._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        self._control._toggles[var].clicked.emit()
        self.assertEqual(self._window.axes()[0].get_ylim(), (-45.0, 945.0))
        self.assertImage('testTime0.png')

        # Change the time
        self._control.plot(time=1)
        self.assertEqual(self._window.axes()[0].get_ylim(), (-5.0, 105.0))
        self.assertImage('testTime1.png')

        # Change to an invalid time, the control should be disabled and plot removed
        self._control.plot(time=2.5)
        self.assertFalse(self._control.isEnabled())
        self.assertImage('testEmpty.png')

    def testRepr(self):
        """
        Test script function.
        """
        self.copyfiles()
        self.create()
        var = 't*x**2'
        self._control._toggles[var].CheckBox.setCheckState(QtCore.Qt.Checked)
        self._control._toggles[var].clicked.emit()
        self._control.plot(time=9.0)

        output, imports = self._control.repr()
        self.assertIn("x = data('index (Peacock)', time=9.0)", output)
        self.assertIn("y = data('t*x**2', time=9.0)", output)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
