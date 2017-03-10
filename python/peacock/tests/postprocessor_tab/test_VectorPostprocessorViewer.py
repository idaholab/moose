#!/usr/bin/env python
import sys
import os
import unittest
import shutil
import glob
from PyQt5 import QtCore, QtWidgets
from peacock.PostprocessorViewer.VectorPostprocessorViewer import VectorPostprocessorViewer
from peacock.utils import Testing


class TestVectorPostprocessorViewer(Testing.PeacockImageTestCase):
    """
    Test class for the ArtistToggleWidget which toggles postprocessor lines.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates the GUI containing the ArtistGroupWidget and the matplotlib figure axes.
        """
        self._filename = "{}_test_*.csv".format(self.__class__.__name__)
        self._widget = VectorPostprocessorViewer(timeout=None)
        self._widget.initialize([self._filename])

    def copyfiles(self):
        """
        Copy to temprary location.
        """
        for i in [0, 1, 3, 5, 7, 9]:
            shutil.copyfile('../input/vpp2_000{}.csv'.format(i), "{}_test_000{}.csv".format(self.__class__.__name__, i))
        for data in self._widget._data[0]:
            data.load()

    def tearDown(self):
        """
        Remove temporary.
        """
        for filename in glob.glob(self._filename):
            if os.path.exists(filename):
                os.remove(filename)

    def write(self, filename):
        """
        Overload the write method.
        """
        self._widget.currentWidget().OutputPlugin.write.emit(filename)

    def plot(self):
        """
        Create plot with all widgets modified.
        """
        widget = self._widget.currentWidget()

        # Plot some data
        toggle = widget.PostprocessorSelectPlugin._groups[0]._toggles['t*x**2']
        toggle.CheckBox.setCheckState(QtCore.Qt.Checked)
        toggle.PlotAxis.setCurrentIndex(1)
        toggle.LineStyle.setCurrentIndex(1)
        toggle.LineWidth.setValue(5)
        toggle.clicked.emit()

        # Add title and legend
        ax = widget.AxesSettingsPlugin
        ax.Title.setText('Data Plot')
        ax.Title.editingFinished.emit()
        ax.Legend2.setCheckState(QtCore.Qt.Checked)
        ax.Legend2.clicked.emit(True)
        ax.Legend2Location.setCurrentIndex(4)
        ax.Legend2Location.currentIndexChanged.emit(4)
        ax.onAxesModified()

        # Set limits and axis titles (y2-only)
        ax = widget.AxisTabsPlugin.Y2AxisTab
        ax.Label.setText('y2y2y2y2y2y2y2y2y2y2y2y2y2')
        ax.Label.editingFinished.emit()
        ax.RangeMinimum.setText('1')
        ax.RangeMinimum.editingFinished.emit()

    def testEmpty(self):
        """
        Test that empty plot is working.
        """
        self.assertImage('testEmpty.png')

    def testWidgets(self):
        """
        Test that the widgets contained in PostprocessorPlotWidget are working.
        """
        self.copyfiles()
        self.plot()
        self.assertImage('testWidgets.png')
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')

    def testCloneClose(self):
        """
        Test clone button works.
        """
        self.copyfiles()
        self._widget.cornerWidget().clone.emit()
        self.assertEqual(self._widget.count(), 2)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertTrue(self._widget.cornerWidget().CloseButton.isEnabled())
        self.assertImage('testEmpty.png')

        # Plot something
        self.plot()
        self.assertImage('testWidgets.png')

        # Switch to first tab
        self._widget.setCurrentIndex(0)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')
        self.assertImage('testEmpty.png')
        self.plot()
        self.assertImage('testWidgets.png')

        # Close the first tab
        self._widget.cornerWidget().close.emit()
        self.assertEqual(self._widget.count(), 1)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
