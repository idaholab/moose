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
import shutil
from PyQt5 import QtWidgets
import mock

from peacock.ExodusViewer.plugins.FilePlugin import main
from peacock.utils import Testing

class TestFilePlugin(Testing.PeacockImageTestCase):
    """
    Testing for FileControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    temp_file = 'TestFilePlugin_test.e'

    @classmethod
    def setUpClass(cls):
        super(TestFilePlugin, cls).setUpClass()
        if os.path.exists(cls.temp_file):
            os.remove(cls.temp_file)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """

        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
        self._filenames.append(self.temp_file)
        self._widget, self._window = main(size=[600,600])

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that things initialize correctly
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self.assertEqual(4, self._widget.FilePlugin.FileList.count())
        self.assertEqual(os.path.basename(self._filenames[0]), str(self._widget.FilePlugin.FileList.currentText()))
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onWindowRequiresUpdate()
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'diffused')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), 'Magnitude')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())
        self.assertImage('testInitial.png')

    def testVariable(self):
        """
        Test changing variables.
        """
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.VariableList.setCurrentIndex(0)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(0)
        self.assertImage('testVariable.png')

    def testChangeFiles(self):
        """
        Test that new files load when selected and save camera state.
        """
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # The current view
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847),
                                     (9.2960, -0.4218, 12.6685),
                                     (0.0000, 0.0000, 0.1250))
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles0.png')

        # Switch files
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles1.png')
        self.assertNotEqual((-0.7786, 0.2277, 0.5847),
                            self._window._result.getVTKRenderer().GetActiveCamera().GetViewUp())

        # Switch back to initial (using same file name as before)
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles0.png')

    def testDelayedFile(self):
        """
        Test that a file that does not exist initially is disabled then available.
        """
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # Index 3 should be disabled
        self._widget.FilePlugin.FileList.showPopup()
        self.assertFalse(self._widget.FilePlugin.FileList.model().item(3).isEnabled())

        # Load the file and check status
        shutil.copyfile(self._filenames[0], self._filenames[3])
        self._window._timers['initialize'].timeout.emit()
        self._widget.FilePlugin.FileList.showPopup()
        self.assertTrue(self._widget.FilePlugin.FileList.model().item(3).isEnabled())
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertImage('testInitial.png')

    @mock.patch.object(QtWidgets.QFileDialog, 'selectedFiles')
    @mock.patch.object(QtWidgets.QFileDialog, 'exec_')
    def testDialogSingleFile(self, mock_exec, mock_open_files):
        """
        Test opening a file with uninitialized plugin.
        """
        mock_exec.return_value = QtWidgets.QFileDialog.Accepted
        mock_open_files.return_value = [self._filenames[0]]
        self.assertFalse(self._window.isEnabled())
        self._widget.FilePlugin.OpenFiles.clicked.emit()

        avail = self._widget.FilePlugin.FileList
        self.assertEqual([self._filenames[0]], [avail.itemData(i) for i in range(avail.count())])
        self.assertEqual([os.path.basename(self._filenames[0])], [avail.itemText(i) for i in range(avail.count())])
        self.assertEqual(avail.count(), 1)
        self.assertEqual(avail.currentText(), os.path.basename(self._filenames[0]))
        self.assertEqual(avail.currentIndex(), 0)
        self.assertTrue(self._window.isEnabled())
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertImage('testInitial.png')

    @mock.patch.object(QtWidgets.QFileDialog, 'selectedFiles')
    @mock.patch.object(QtWidgets.QFileDialog, 'exec_')
    def testDialogMultipleFiles(self, mock_exec, mock_open_files):
        """
        Test opening many files with uninitialized plugin.
        """
        shutil.copyfile(self._filenames[0], self._filenames[3])

        mock_exec.return_value = QtWidgets.QFileDialog.Accepted
        mock_open_files.return_value = self._filenames
        self._widget.FilePlugin.OpenFiles.clicked.emit()

        avail = self._widget.FilePlugin.FileList
        gold = [str(self._widget.FilePlugin.FileList.itemData(i)) for i in range(self._widget.FilePlugin.FileList.count())]
        self.assertEqual(self._filenames, gold)
        self.assertEqual([os.path.basename(f) for f in self._filenames], [avail.itemText(i) for i in range(avail.count())])
        self.assertEqual(avail.count(), 4)
        self.assertEqual(avail.currentText(), os.path.basename(self._filenames[3]))
        self.assertEqual(avail.currentIndex(), 3)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertImage('testInitial.png')

    def testVector(self):
        """
        Test changing vector stuff.
        """
        # Change the file to something with vectors
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(os.path.basename(self._filenames[1]),
                         str(self._widget.FilePlugin.FileList.currentText()))
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), "vel_")
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "Magnitude")
        self.assertImage('testVectorInitial.png')

        # Change to x
        self._widget.FilePlugin.ComponentList.setCurrentIndex(1)
        self._widget.FilePlugin.ComponentList.currentIndexChanged.emit(1)
        self.assertEqual(self._window._result.getOption('component'), 0)
        self.assertImage('testVectorX.png')

        # Change to y
        self._widget.FilePlugin.ComponentList.setCurrentIndex(2)
        self._widget.FilePlugin.ComponentList.currentIndexChanged.emit(2)
        self.assertEqual(self._window._result.getOption('component'), 1)
        self.assertImage('testVectorY.png')

    def testState(self):
        """
        Test changing state storing and loading.
        """
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # Check initial state
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'aux_elem')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())
        self.assertImage('testStateInitial.png')

        # Change to convected
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'convected')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentData(), -1)
        self.assertImage('testConvected.png')

        # Change to vector
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'u')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())

        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'vel_')
        self.assertTrue(self._widget.FilePlugin.ComponentList.isEnabled())

        self._widget.FilePlugin.ComponentList.setCurrentIndex(2)
        self._widget.FilePlugin.ComponentList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), 'y')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentData(), 1)

        self.assertImage('testStateVectorY.png')

        # Change back
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'convected')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentData(), -1)
        self.assertImage('testConvected.png')

    def testState2(self):
        """
        Additional state checking.
        """
        # Change the file to something with vectors
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # Check default state
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'aux_elem')
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())

        # Change to vector
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'vel_')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "Magnitude")
        self.assertTrue(self._widget.FilePlugin.ComponentList.isEnabled())
        self._widget.FilePlugin.ComponentList.setCurrentIndex(1)
        self._widget.FilePlugin.ComponentList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "x")
        self.assertEqual(self._window._result.getOption('component'), 0)

        # Change back to mug
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'aux_elem')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "Magnitude")
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())

        # Change variable
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'convected')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "Magnitude")
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())

        # Change back to vector
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'vel_')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "x")
        self.assertEqual(self._window._result.getOption('component'), 0)
        self.assertTrue(self._widget.FilePlugin.ComponentList.isEnabled())

        # Change back to mug
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'convected')
        self.assertEqual(self._widget.FilePlugin.ComponentList.currentText(), "Magnitude")
        self.assertFalse(self._widget.FilePlugin.ComponentList.isEnabled())

class TestFilePluginNewVariable(Testing.PeacockImageTestCase):
    """
    Testing for FilePlugin widget when a new variable is added.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    temp_file = 'TestFilePluginNewVariable_test.e'

    @classmethod
    def setUpClass(cls):
        super(TestFilePluginNewVariable, cls).setUpClass()
        if os.path.exists(cls.temp_file):
            os.remove(cls.temp_file)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """
        self._filenames = Testing.get_chigger_input_list('simple_diffusion_out.e',
                                                         'simple_diffusion_new_var_out.e')
        self._widget, self._window = main(size=[600,600])

    def testInitial(self):
        shutil.copy(self._filenames[0], self.temp_file)
        self._widget.FilePlugin.onSetFilenames([self.temp_file])
        self.assertEqual(self._widget.FilePlugin.VariableList.count(), 2)
        self.assertEqual(self._widget.FilePlugin.VariableList.itemText(0), 'aux')
        self.assertEqual(self._widget.FilePlugin.VariableList.itemText(1), 'u')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'aux')

        Testing.process_events(2)
        shutil.copy(self._filenames[1], self.temp_file)
        self._window.onWindowRequiresUpdate()
        Testing.process_events(2)
        self.assertEqual(self._widget.FilePlugin.VariableList.count(), 3)
        self.assertEqual(self._widget.FilePlugin.VariableList.itemText(0), 'aux')
        self.assertEqual(self._widget.FilePlugin.VariableList.itemText(1), 'New_0')
        self.assertEqual(self._widget.FilePlugin.VariableList.itemText(2), 'u')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'aux')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
