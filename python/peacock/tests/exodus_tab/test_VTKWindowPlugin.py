#!/usr/bin/env python
import sys
import os
import unittest
import glob
import shutil
import vtk
import time
from PyQt5 import QtWidgets

import chigger
from peacock.ExodusViewer.plugins.VTKWindowPlugin import main
from peacock.utils import Testing


class TestVTKWindowPlugin(Testing.PeacockImageTestCase):
    """
    Testing for VTKWindowPlugin
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filename = Testing.get_chigger_input('mug_blocks_out.e')

    #: str: Temporary filename for testing delayed load (see testFilename)
    _temp_file = 'TestVTKWindowPlugin.e'

    @classmethod
    def setUpClass(cls):
        super(TestVTKWindowPlugin, cls).setUpClass()

        if os.path.exists(cls._temp_file):
            os.remove(cls._temp_file)

    def setUp(self):
        """
        Loads an Exodus file in the VTKWindowWidget object using a structure similar to the ExodusViewer widget.
        """
        self.sleepIfSlow()
        self._widget, self._window = main(size=[600,600])

    def testInitialize(self):
        """
        Test the result open and are initialized.
        """
        self._window.onFileChanged(self._filename)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertTrue(self._window._initialized)
        self.assertImage('testInitialize.png', allowed=0.98)

    def testCamera(self):
        """
        Test that the camera can be modified.
        """
        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onFileChanged(self._filename)
        self._window.onCameraChanged(camera)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertEqual(camera.GetViewUp(), self._window._result.getVTKRenderer().GetActiveCamera().GetViewUp())
        self.assertImage('testCamera.png')

    def testReader(self):
        """
        Test that reader settings may be changed.
        """
        self._window.onFileChanged(self._filename)
        self._window._reader.setOptions(timestep=1)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        tdata = self._window._reader.getTimeData()
        self.assertEqual(1, tdata.timestep)
        self.assertEqual(0.1, tdata.time)
        self.assertImage('testReader.png')

    def testResult(self):
        """
        Test that result settings may be changed.
        """
        self._window.onFileChanged(self._filename)
        self._window._result.setOptions(cmap='viridis')
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()
        self.assertEqual('viridis', self._window._result.getOption('cmap'))
        self.assertImage('testResult.png')

    def testHighlight(self):
        """
        Test the highlighting is working.
        """
        self._window.onFileChanged(self._filename)
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        self._window.onHighlight(block=['76'])
        self.assertImage('testHighlightOn.png')

        self._window.onHighlight()
        self.assertImage('testHighlightOff.png')

    def testFilename(self):
        """
        Tests that non-existent files, new files, and removed files do not break window.
        """

        # The source and destination filenames
        filename = Testing.get_chigger_input('step10_micro_out.e')
        newfile = self._temp_file

        # Remove any existing files
        for fname in glob.glob(newfile + '*'):
            os.remove(fname)

        # Supply a non-existent file
        self._window.onFileChanged(newfile)
        self.assertImage('testFilenameEmpty.png')

        # Create the files and simulate the initialization timer timeout call
        shutil.copy(filename, newfile)
        for i in range(2, 6):
            ext = '-s00' + str(i)
            shutil.copy(filename + ext, newfile + ext)
        time.sleep(1.5) # sleep so modified times differ
        self._window._timers["initialize"].timeout.emit()
        self._window.onResultOptionsChanged({'variable':'phi'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testFilenameCreated.png', allowed=0.98)

        # Add new files and simulate the update timer timeout call
        for i in range(6, 10):
            ext = '-s00' + str(i)
            shutil.copy(filename + ext, newfile + ext)
        self._window.onWindowRequiresUpdate()
        self.assertImage('testFilenameUpdated.png', allowed=0.98)

        # Remove the files and simulate a call to the update timer
        for fname in glob.glob(newfile + '*'):
            os.remove(fname)
        self._window.onWindowRequiresUpdate()
        self.assertImage('testFilenameEmpty.png') # the window should be empty again

    def testIteractorStyle(self):
        """
        Tests interaction style matches the mesh dimensionality
        """
        self._window.onFileChanged(self._filename)
        self.assertIsNone(self._window._window.getOption('style'))
        self.assertIsInstance(self._window._window.getVTKInteractor().GetInteractorStyle(), chigger.base.KeyPressInteractorStyle)

        self._window.onFileChanged(Testing.get_chigger_input('displace.e'))
        self.assertIsNone(self._window._window.getOption('style'))
        self.assertIsInstance(self._window._window.getVTKInteractor().GetInteractorStyle(), vtk.vtkInteractorStyleImage)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
