#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
from PyQt5 import QtCore, QtWidgets

from peacock.ExodusViewer.plugins.GoldDiffPlugin import main
from peacock.utils import Testing

class TestGoldDiffPlugin(Testing.PeacockImageTestCase):
    """
    Testing for GoldDiffPlugin widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin and GoldDiffPlugin widget.
        """

        self.sleepIfSlow()
        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e')
        self._widget, self._window = main(size=[400,400])
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)

    def write(self, filename, **kwargs):
        """
        Write the image file to be compared.

        NOTE: Requires self._window to be a VTKWindowWidget or FigureWidget object.
        """
        window = kwargs.pop('window', self._window)
        window.onWrite(filename, **kwargs)

    def testInitial(self):
        """
        Test the initial state.
        """
        self.assertEqual(2, self._widget.FilePlugin.FileList.count())
        self.assertEqual(os.path.basename(self._filenames[0]),
                         str(self._widget.FilePlugin.FileList.currentText()))
        self.assertImage('testInitial.png')

    def testGold(self):
        """
        Test the gold toggle is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.isEnabled())
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)
        if self._window.devicePixelRatio() == 1:
            self.assertImage('testGold.png', window=self._widget.GoldDiffPlugin.GoldVTKWindow)

    def testDiff(self):
        """
        Test the diff toggle is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.isEnabled())
        self._widget.GoldDiffPlugin.DiffToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.DiffToggle.clicked.emit(True)
        if self._window.devicePixelRatio() == 1:
            self.assertImage('testDiff.png', window=self._widget.GoldDiffPlugin.DiffVTKWindow)

    def testCameraLink(self):
        """
        Test the camera link is working.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.LinkToggle.isChecked())
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)
        self.assertIsNotNone(self._widget.GoldDiffPlugin._gold_observer)
        self._widget.GoldDiffPlugin.LinkToggle.setChecked(QtCore.Qt.Unchecked)
        self._widget.GoldDiffPlugin.LinkToggle.clicked.emit(False)
        self._widget.GoldDiffPlugin.GoldVTKWindow._window.update()
        self.assertIsNone(self._widget.GoldDiffPlugin._gold_observer)

    def testDisabled(self):
        """
        Test the gold/diff window gets disabled when a file doesn't have gold.
        """
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.GoldDiffPlugin.isVisible())

    def testState(self):
        """
        Test the state storing of Gold/Diff windows.
        """
        self.assertTrue(self._widget.GoldDiffPlugin.LinkToggle.isChecked())
        self.assertFalse(self._widget.GoldDiffPlugin.GoldToggle.isChecked())

        self._widget.GoldDiffPlugin.LinkToggle.setChecked(QtCore.Qt.Unchecked)
        self._widget.GoldDiffPlugin.LinkToggle.clicked.emit(False)
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)

        self.assertFalse(self._widget.GoldDiffPlugin.LinkToggle.isChecked())
        self.assertTrue(self._widget.GoldDiffPlugin.GoldToggle.isChecked())

        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.GoldDiffPlugin.isVisible())

        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertTrue(self._widget.GoldDiffPlugin.isVisible())
        self.assertFalse(self._widget.GoldDiffPlugin.LinkToggle.isChecked())
        self.assertTrue(self._widget.GoldDiffPlugin.GoldToggle.isChecked())

    def testVariable(self):
        """
        Test that changing variable propagates.
        """
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)
        if self._window.devicePixelRatio() == 1:
            self.assertImage('testChangeVariable.png', window=self._widget.GoldDiffPlugin.GoldVTKWindow)

    def testChangeFileWhenSelected(self):
        """
        Test that window closes when file is toggled to a file w/o a gold
        """
        self._widget.GoldDiffPlugin.GoldToggle.setChecked(QtCore.Qt.Checked)
        self._widget.GoldDiffPlugin.GoldToggle.clicked.emit(True)
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.GoldDiffPlugin.isVisible())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
