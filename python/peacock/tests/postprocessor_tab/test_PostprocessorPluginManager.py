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
import subprocess
from PyQt5 import QtCore, QtWidgets

from peacock.PostprocessorViewer.PostprocessorDataWidget import PostprocessorDataWidget
from peacock.PostprocessorViewer.PostprocessorPluginManager import main
from peacock.utils import Testing
import mooseutils


class TestPostprocessorPluginManager(Testing.PeacockImageTestCase):
    """
    Test class for the ArtistToggleWidget which toggles postprocessor lines.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    @classmethod
    def setUpClass(cls):
        """
        Clean up from previous testing.
        """
        super(TestPostprocessorPluginManager, cls).setUpClass()

        names = ['{}_test_script.py'.format(cls.__name__), '{}_test_output.pdf'.format(cls.__name__),  '{}_test_output.png'.format(cls.__name__)]
        for name in names:
            if os.path.exists(name):
                os.remove(name)

    def setUp(self):
        """
        Creates the GUI containing the ArtistGroupWidget and the matplotlib figure axes.
        """
        import matplotlib
        matplotlib.rcParams["figure.figsize"] = (5., 5.)
        matplotlib.rcParams["figure.dpi"] = (100)

        data = [PostprocessorDataWidget(mooseutils.PostprocessorReader('../input/white_elephant_jan_2016.csv'))]
        self._widget, self._window = main()
        self._widget.FigurePlugin.setFixedSize(QtCore.QSize(500, 500))
        self._widget.call('onSetData', data)

    def plot(self):
        """
        Create plot with all widgets modified.
        """

        # Plot some data
        toggle = self._widget.PostprocessorSelectPlugin._groups[0]._toggles['air_temp_set_1']
        toggle.CheckBox.setCheckState(QtCore.Qt.Checked)
        toggle.PlotAxis.setCurrentIndex(1)
        toggle.LineStyle.setCurrentIndex(1)
        toggle.LineWidth.setValue(5)
        toggle.clicked.emit()

        # Add title and legend
        ax = self._widget.AxesSettingsPlugin
        ax.Title.setText('Snow Data')
        ax.Title.editingFinished.emit()
        ax.Legend2.setCheckState(QtCore.Qt.Checked)
        ax.Legend2.clicked.emit(True)
        ax.Legend2Location.setCurrentIndex(4)
        ax.Legend2Location.currentIndexChanged.emit(4)
        ax.onAxesModified()

        # Set limits and axis titles (y2-only)
        ax = self._widget.AxisTabsPlugin.Y2AxisTab
        ax.Label.setText('Air Temperature [C]')
        ax.Label.editingFinished.emit()
        ax.RangeMinimum.setText('0')
        ax.RangeMinimum.editingFinished.emit()

    def testWidgets(self):
        """
        Test that the widgets contained in PostprocessorPlotWidget are working.
        """
        self.plot()
        self.assertImage('testWidgets.png')

    @unittest.skip("Broken by #12702")
    def testOutput(self):
        """
        Test that the python output is working.
        """

        self.plot()

        # Write the python script
        output = self._widget.OutputPlugin
        name = '{}_test_script.py'.format(self.__class__.__name__)
        output.write.emit(name)
        self.assertTrue(os.path.exists(name))

        # Compare with gold
        with open(name, 'r') as fid:
            script = fid.read()
        with open(os.path.join('gold', name), 'r') as fid:
            gold_script = fid.read()
        self.assertIn(script.strip('\n'), gold_script.strip('\n'))

        # Remove the show from the script and make it output a png
        script = script.replace('plt.show()', '')
        script = script.replace('output.pdf', 'output.png')
        with open(name, 'w') as fid:
            fid.write(script)
        subprocess.call(['python', name], stdout=open(os.devnull, 'wb'), stderr=subprocess.STDOUT)
        self.assertTrue(os.path.exists('output.png'))
        differ = mooseutils.ImageDiffer(os.path.join('gold', 'output.png'), 'output.png', allowed=0.99)
        print(differ.message())
        self.assertFalse(differ.fail(), "{} does not match the gold file.".format(name))

        # Test pdf output
        name = '{}_test_output.pdf'.format(self.__class__.__name__)
        output.write.emit(name)
        self.assertTrue(os.path.exists(name))

        # Test png output
        name = '{}_test_output.png'.format(self.__class__.__name__)
        output.write.emit(name)
        self.assertTrue(os.path.exists(name))
        goldname = os.path.join('gold', name)
        differ = mooseutils.ImageDiffer(goldname, name, allowed=0.99)
        self.assertFalse(differ.fail(), "{} does not match the gold file.".format(name))

    def testLiveScript(self):
        """
        Tests that live script widget.
        """
        self._widget.OutputPlugin.LiveScriptButton.clicked.emit()
        self.assertTrue(self._widget.OutputPlugin.LiveScript.isVisible())
        self.assertIn("plt.figure", self._widget.OutputPlugin.LiveScript.toPlainText())

        self.plot()
        self._widget.OutputPlugin.onAxesModified()
        self.assertIn("markersize=1", self._widget.OutputPlugin.LiveScript.toPlainText())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
