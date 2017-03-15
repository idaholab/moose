#!/usr/bin/env python
import os
import sys
import unittest
import shutil
from PyQt5 import QtWidgets
import vtk
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
        self._widget.initialize(self._filenames)
        self.assertEqual(4, self._widget.FilePlugin.AvailableFiles.count())
        self.assertEqual(os.path.basename(self._filenames[0]), str(self._widget.FilePlugin.AvailableFiles.currentText()))
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testInitial.png')

    def testChangeFiles(self):
        """
        Test that new files load when selected and save camera state.
        """
        self._widget.initialize(self._filenames)

        # Setup a camera
        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)

        # The current view
        self.assertEqual(camera.GetViewUp(), self._window._result.getVTKRenderer().GetActiveCamera().GetViewUp())
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles0.png')

        # Switch files
        self._widget.FilePlugin.AvailableFiles.setCurrentIndex(1)
        self._window.onResultOptionsChanged({'variable':'vel_'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles1.png')
        self.assertNotEqual(camera.GetViewUp(), self._window._result.getVTKRenderer().GetActiveCamera().GetViewUp())

        # Switch back to initial (using same file name as before)
        self._widget.FilePlugin.AvailableFiles.setCurrentIndex(0)
        self.assertEqual(camera.GetViewUp(), self._window._result.getVTKRenderer().GetActiveCamera().GetViewUp())
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testChangeFiles0.png')

    def testDelayedFile(self):
        """
        Test that a file that does not exist initially is disabled then available.
        """
        self._widget.initialize(self._filenames)

        # Index 3 should be disabled
        self._widget.FilePlugin.AvailableFiles.showPopup()
        self.assertFalse(self._widget.FilePlugin.AvailableFiles.model().item(3).isEnabled())

        # Load the file and check status
        shutil.copyfile(self._filenames[0], self._filenames[3])
        self._window._timers['initialize'].timeout.emit()
        self._widget.FilePlugin.AvailableFiles.showPopup()
        self.assertTrue(self._widget.FilePlugin.AvailableFiles.model().item(3).isEnabled())
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
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

        avail = self._widget.FilePlugin.AvailableFiles
        self.assertEqual([self._filenames[0]], [avail.itemData(i) for i in range(avail.count())])
        self.assertEqual([os.path.basename(self._filenames[0])], [avail.itemText(i) for i in range(avail.count())])
        self.assertEqual(avail.count(), 1)
        self.assertEqual(avail.currentText(), os.path.basename(self._filenames[0]))
        self.assertEqual(avail.currentIndex(), 0)
        self.assertTrue(self._window.isEnabled())
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self.assertImage('testInitial.png')

    @mock.patch.object(QtWidgets.QFileDialog, 'selectedFiles')
    @mock.patch.object(QtWidgets.QFileDialog, 'exec_')
    def testDialogMultipleFiles(self, mock_exec, mock_open_files):
        """
        Test opening many files with uninitialized plugin.
        """
        mock_exec.return_value = QtWidgets.QFileDialog.Accepted
        mock_open_files.return_value = self._filenames
        self._widget.FilePlugin.OpenFiles.clicked.emit()

        avail = self._widget.FilePlugin.AvailableFiles
        self.assertEqual(self._filenames, [avail.itemData(i) for i in range(avail.count())])
        self.assertEqual([os.path.basename(f) for f in self._filenames], [avail.itemText(i) for i in range(avail.count())])
        self.assertEqual(avail.count(), 4)
        self.assertEqual(avail.currentText(), os.path.basename(self._filenames[3]))
        self.assertEqual(avail.currentIndex(), 3)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self.assertImage('testInitial.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
