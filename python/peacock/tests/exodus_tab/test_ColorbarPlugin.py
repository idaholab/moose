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
from peacock.ExodusViewer.plugins.ColorbarPlugin import main
from peacock.utils import Testing

class TestColorbarPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """

        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
        self._widget, self._window = main(size=[400,400])
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # Start with 'diffused' variable
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)


    def testInitial(self):
        """
        Test the initial state of the widget.
        """
        self.assertFalse(self._widget.ColorbarPlugin.RangeMinimumMode.isChecked())
        self.assertFalse(self._widget.ColorbarPlugin.RangeMinimum.isEnabled())
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.0')
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())

        self.assertFalse(self._widget.ColorbarPlugin.RangeMaximumMode.isChecked())
        self.assertFalse(self._widget.ColorbarPlugin.RangeMaximum.isEnabled())
        #self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '2.0')
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())

        self.assertEqual(self._widget.ColorbarPlugin.ColorMapList.currentText(), 'default')
        self.assertFalse(self._widget.ColorbarPlugin.ColorMapReverse.isChecked())
        self.assertTrue(self._widget.ColorbarPlugin.ColorBarToggle.isChecked())
        self.assertImage('testInitial.png', allowed=0.96)

    def testRange(self):
        """
        Test that custom range is operational.
        """

        # Minimum
        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Checked)

        self.assertTrue(self._widget.ColorbarPlugin.RangeMinimumMode.isChecked())
        self.assertFalse(self._widget.ColorbarPlugin.RangeMaximumMode.isChecked())
        self._widget.ColorbarPlugin.RangeMinimum.setText(str(0.15))
        self._widget.ColorbarPlugin.RangeMinimum.editingFinished.emit()
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())
        self.assertImage('testRangeMinimum.png', allowed=0.96)

        # Maximum
        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Checked)

        self.assertFalse(self._widget.ColorbarPlugin.RangeMinimumMode.isChecked())
        self.assertTrue(self._widget.ColorbarPlugin.RangeMaximumMode.isChecked())
        self._widget.ColorbarPlugin.RangeMaximum.setText(str(1.5))
        self._widget.ColorbarPlugin.RangeMaximum.editingFinished.emit()
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())
        self.assertImage('testRangeMaximum.png', allowed=0.96)

        # Min/Max
        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Checked)

        self._widget.ColorbarPlugin.RangeMinimum.setText(str(0.5))
        self._widget.ColorbarPlugin.RangeMinimum.editingFinished.emit()
        self._widget.ColorbarPlugin.RangeMaximum.setText(str(1.5))
        self._widget.ColorbarPlugin.RangeMaximum.editingFinished.emit()
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.5')
        self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '1.5')
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())
        self.assertImage('testRangeMinMaximum.png', allowed=0.96)

    def testColormap(self):
        """
        Test that colormap can be changed and reversed.
        """

        index = self._widget.ColorbarPlugin.ColorMapList.findText('viridis')
        self._widget.ColorbarPlugin.ColorMapList.setCurrentIndex(index)
        self._widget.ColorbarPlugin.ColorMapList.currentIndexChanged.emit(index)
        self.assertImage('testColorMap.png', allowed=0.96)

        # Reverse
        self._widget.ColorbarPlugin.ColorMapReverse.setCheckState(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.ColorMapReverse.clicked.emit(QtCore.Qt.Checked)
        self.assertImage('testColorMapReversed.png', allowed=0.96)

        # Test going back works
        self._widget.ColorbarPlugin.ColorMapReverse.setCheckState(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.ColorMapReverse.clicked.emit(QtCore.Qt.Unchecked)
        self.assertImage('testColorMap.png', allowed=0.96)

    def testColorBar(self):
        """
        Test that the colorbar can be disabled.
        """
        self.assertImage('testInitial.png', allowed=0.96)
        self._widget.ColorbarPlugin.ColorBarToggle.setCheckState(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.ColorBarToggle.clicked.emit(QtCore.Qt.Unchecked)
        self.assertImage('testColorBarOff.png')
        self._widget.ColorbarPlugin.ColorBarToggle.setCheckState(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.ColorBarToggle.clicked.emit(QtCore.Qt.Checked)
        self.assertImage('testInitial.png', allowed=0.96)

    def testAutoRangeUpdate(self):
        """
        Test that the range updates when the time changes.
        """
        self._widget.VTKWindowPlugin._reader.update(timestep=0)
        self._widget.VTKWindowPlugin.onWindowRequiresUpdate()
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.0')
        self.assertAlmostEqual(float(self._widget.ColorbarPlugin.RangeMaximum.text()), 0.0)
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())

    def testFileChangedState(self):
        """
        Test changing files and making sure the state is saved/restored
        """
        fp = self._widget.FilePlugin

        # Change state of everything
        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Checked)

        self._widget.ColorbarPlugin.RangeMinimum.setText(str(0.5))
        self._widget.ColorbarPlugin.RangeMinimum.editingFinished.emit()
        self._widget.ColorbarPlugin.RangeMaximum.setText(str(1.5))
        self._widget.ColorbarPlugin.RangeMaximum.editingFinished.emit()

        index = self._widget.ColorbarPlugin.ColorMapList.findText('viridis')
        self._widget.ColorbarPlugin.ColorMapList.setCurrentIndex(index)
        self._widget.ColorbarPlugin.ColorMapList.currentIndexChanged.emit(index)

        self._widget.ColorbarPlugin.ColorBarToggle.setCheckState(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.ColorBarToggle.clicked.emit(QtCore.Qt.Unchecked)

        self._widget.ColorbarPlugin.ColorMapReverse.setCheckState(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.ColorMapReverse.clicked.emit(QtCore.Qt.Checked)

        # Change file
        fp.FileList.setCurrentIndex(1)
        fp.FileList.currentIndexChanged.emit(1)

        # Check state
        self.assertFalse(self._widget.ColorbarPlugin.RangeMinimumMode.isChecked())
        self.assertFalse(self._widget.ColorbarPlugin.RangeMaximumMode.isChecked())
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), "0.0")
        #self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), "1.0")
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())
        self.assertIn('color:#8C8C8C', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())
        self.assertEqual(self._widget.ColorbarPlugin.ColorMapList.currentText(), 'default')
        self.assertTrue(self._widget.ColorbarPlugin.ColorBarToggle.isChecked())
        self.assertFalse(self._widget.ColorbarPlugin.ColorMapReverse.isChecked())

        # Change file back
        fp.FileList.setCurrentIndex(0)
        fp.FileList.currentIndexChanged.emit(0)
        #self._window.render()

        self.assertTrue(self._widget.ColorbarPlugin.RangeMinimumMode.isChecked())
        self.assertTrue(self._widget.ColorbarPlugin.RangeMaximumMode.isChecked())
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), "0.5")
        self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), "1.5")
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMinimum.styleSheet())
        self.assertIn('color:#000000', self._widget.ColorbarPlugin.RangeMaximum.styleSheet())
        self.assertEqual(self._widget.ColorbarPlugin.ColorMapList.currentText(), 'viridis')
        self.assertFalse(self._widget.ColorbarPlugin.ColorBarToggle.isChecked())
        self.assertTrue(self._widget.ColorbarPlugin.ColorMapReverse.isChecked())

    def testMinMaxState(self):
        """
        Test state change with changing min/max toggles.
        """
        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.0')
        #self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '2.0')

        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Checked)

        self._widget.ColorbarPlugin.RangeMinimum.setText(str(0.5))
        self._widget.ColorbarPlugin.RangeMinimum.editingFinished.emit()
        self._widget.ColorbarPlugin.RangeMaximum.setText(str(1.5))
        self._widget.ColorbarPlugin.RangeMaximum.editingFinished.emit()

        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.5')
        self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '1.5')

        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Unchecked)

        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.0')
        #self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '2.0')

        self._widget.ColorbarPlugin.RangeMinimumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMinimumMode.clicked.emit(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.setChecked(QtCore.Qt.Checked)
        self._widget.ColorbarPlugin.RangeMaximumMode.clicked.emit(QtCore.Qt.Checked)

        self.assertEqual(self._widget.ColorbarPlugin.RangeMinimum.text(), '0.5')
        self.assertEqual(self._widget.ColorbarPlugin.RangeMaximum.text(), '1.5')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
