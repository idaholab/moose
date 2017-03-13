#!/usr/bin/env python
import os
import sys
import unittest
import shutil
from PyQt5 import QtWidgets
import vtk

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
        self._widget.initialize(self._filenames)

    def testInitial(self):
        """
        Test the initial state.
        """

        # Test that things initialize correctly
        self.assertEqual(4, self._widget.FilePlugin.AvailableFiles.count())
        self.assertEqual(os.path.basename(self._filenames[0]), str(self._widget.FilePlugin.AvailableFiles.currentText()))
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testInitial.png')

    def testChangeFiles(self):
        """
        Test that new files load when selected and save camera state.
        """

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


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
