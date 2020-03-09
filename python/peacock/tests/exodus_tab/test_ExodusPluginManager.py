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

from peacock.ExodusViewer.ExodusPluginManager import main
from peacock.utils import Testing
import mooseutils


class TestExodusPluginManager(Testing.PeacockImageTestCase):
    """
    Testing for ExodusPluginManager
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'displace.e')

    #: str: The script file created for testing
    _repr_script = 'TestExodusPluginManager_repr.py'


    @classmethod
    def setUpClass(cls):
        super(TestExodusPluginManager, cls).setUpClass()

        if os.path.exists(cls._repr_script):
            os.remove(cls._repr_script)

    def setUp(self):
        """
        Loads an Exodus file in the VTKWindowWidget object using a structure similar to the ExodusViewer widget.
        """
        self._widget, self._main_window = main(size=[600,600])
        self._window = self._widget.VTKWindowPlugin
        self._widget.FilePlugin.onSetFilenames(self._filenames)

        # Start with 'diffused' variable
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)

    def testInitial(self):
        """
        Tests the file loads.
        """
        self.assertImage('testInitial.png', allowed=0.97)

    def testWidget(self):
        """
        Tests that a bunch of stuff can change without crashing.
        """
        # File
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)

        # Variable
        self._widget.ColorbarPlugin.ColorBarToggle.setCheckState(QtCore.Qt.Unchecked)
        self._widget.ColorbarPlugin.ColorBarToggle.clicked.emit(False)

        # Time
        self._widget.MediaControlPlugin.TimeDisplay.setText('0.26')
        self._widget.MediaControlPlugin.TimeDisplay.editingFinished.emit()
        #self.assertEqual(self._widget.MediaControlPlugin.TimeDisplay.text(), '0.25')

        # Reset
        self._widget.CameraPlugin.ResetButton.clicked.emit()

        # Clip
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)

        # Background
        self._widget.BackgroundPlugin.GradientToggle.setChecked(True)

        # Mesh
        self._widget.MeshPlugin.DisplacementMagnitude.setValue(0.3)
        self._widget.MeshPlugin.DisplacementMagnitude.editingFinished.emit()
        self._widget.MeshPlugin.ScaleY.setValue(1.5)
        self._widget.MeshPlugin.ScaleY.editingFinished.emit()

        self.assertImage('testWidget.png')

    def testPython(self):
        """
        Test generic python script.
        """
        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))
        imagename = '{}_{}'.format(self.__class__.__name__, 'basic.png')
        self.python(imagename)

    def testPythonContour(self):
        """
        Test python script with contours.
        """
        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))
        self._widget.ContourPlugin.setChecked(True)
        self._widget.ContourPlugin.clicked.emit(True)
        imagename = '{}_{}'.format(self.__class__.__name__, 'contour.png')
        self.python(imagename)

    def testPythonClip(self):
        """
        Test python script with clip.
        """

        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        imagename = '{}_{}'.format(self.__class__.__name__, 'clip.png')
        self.python(imagename)

    def testPythonScale(self):
        """
        Test python script with scale.
        """

        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))
        self._widget.MeshPlugin.ScaleY.setValue(1.5)
        self._widget.MeshPlugin.ScaleY.editingFinished.emit()
        imagename = '{}_{}'.format(self.__class__.__name__, 'scale.png')
        self.python(imagename)

    def testPythonMulitple(self):
        """
        Test python script with multiple filters.
        """

        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))
        self._widget.ClipPlugin.setChecked(True)
        self._widget.ClipPlugin.clicked.emit(True)
        self._widget.ClipPlugin.ClipDirection.setCurrentIndex(1)
        self._widget.MeshPlugin.ScaleY.setValue(.25)
        self._widget.MeshPlugin.ScaleY.editingFinished.emit()
        imagename = '{}_{}'.format(self.__class__.__name__, 'multiple.png')
        self.python(imagename)

    def python(self, imagename):
        """
        Test script writer.
        """

        # Test that the script is created
        self._window._window.setOptions(test=True, size=[600,600])
        self._widget.OutputPlugin.write.emit(self._repr_script)
        self.assertTrue(os.path.exists(self._repr_script))

        # Inject write command
        with open(self._repr_script, 'a') as fid:
            fid.write('\nwindow.write({})'.format(repr(imagename)))

        # Execute the script
        subprocess.call(['python', self._repr_script], stdout=open(os.devnull, 'wb'), stderr=subprocess.STDOUT)
        self.assertTrue(os.path.exists(imagename))

        # Diff the image from the script
        differ = mooseutils.ImageDiffer(os.path.join('gold', imagename), imagename)
        print(differ.message())
        self.assertFalse(differ.fail())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
