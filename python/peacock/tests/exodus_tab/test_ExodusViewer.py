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
from PyQt5 import QtWidgets, QtCore

from peacock.ExodusViewer.ExodusViewer import main
from peacock.utils import Testing, qtutils
from mooseutils import message

class TestExodusViewer(Testing.PeacockImageTestCase):
    """
    Testing for ExodusViewer.

    TODO: There is a rendering artifact that shows up in these tests on linux,
          so the imagediffs are not performed.
    """

    #: QApplication: The main App for QT, this must be static to work correctly.
    qapp = QtWidgets.QApplication(sys.argv)

    #: str: The filename to load.
    _filename = Testing.get_chigger_input('mug_blocks_out.e')

    def setUp(self):
        """
        Loads an Exodus file in the VTKWindowWidget object using a structure similar to the ExodusViewer widget.
        """
        message.MOOSE_TESTING_MODE = True
        qtutils.setAppInformation("peacock_exodusviewer")

        settings = QtCore.QSettings()
        settings.clear()
        settings.sync()

        self._widget, self._main_window = main(size=[400,400])
        self._widget.onSetFilenames([self._filename])

        # Start with 'diffused' variable
        self._widget.currentWidget().FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.currentWidget().FilePlugin.VariableList.currentIndexChanged.emit(2)


    def write(self, filename):
        """
        Overload the write method.
        """
        self._widget.currentWidget().OutputPlugin.write.emit(filename)

    def testInitial(self):
        """
        Test initial.
        """
        if sys.platform == 'darwin':
            self.assertImage('testInitial.png', allowed=0.96)
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')

    def testCloneClose(self):
        """
        Test clone button works.
        """
        self._widget.cornerWidget().clone.emit()
        self._widget.currentWidget().FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.currentWidget().FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.count(), 2)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertTrue(self._widget.cornerWidget().CloseButton.isEnabled())

        # Change camera on cloned tab
        self._widget.currentWidget().VTKWindowPlugin.onCameraChanged((-0.7786, 0.2277, 0.5847),
                                                                     (9.2960, -0.4218, 12.6685),
                                                                     (0.0000, 0.0000, 0.1250))
        # Switch to first tab
        self._widget.setCurrentIndex(0)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')

        # Close the first tab
        self._widget.cornerWidget().close.emit()
        self.assertEqual(self._widget.count(), 1)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())

    def testMeshState(self):
        """
        Test that the state of the mesh widget after changing files.
        """
        f0 = Testing.get_chigger_input('diffusion_1.e')
        f1 = Testing.get_chigger_input('diffusion_2.e')
        self._widget.onSetFilenames([f0, f1])
        mesh = self._widget.currentWidget().MeshPlugin
        fp = self._widget.currentWidget().FilePlugin
        fp._callbackFileList(0)
        mesh.ViewMeshToggle.setChecked(False)
        mesh.ScaleX.setValue(.9)
        mesh.ScaleY.setValue(.8)
        mesh.ScaleZ.setValue(.7)
        mesh.Representation.setCurrentIndex(1)
        mesh.DisplacementToggle.setChecked(True)
        mesh.DisplacementMagnitude.setValue(2.0)
        #self.assertImage('testDiffusion1.png') # TODO: The data in the file(s) doesn't line up with gold

        fp._callbackFileList(1)
        # had a case where switching files that had the same variable name
        # disabled the entire widget. Couldn't reproduce it with just the MeshPlugin
        # unit tests.
        self.assertEqual(mesh.isEnabled(), True)
        self.assertEqual(mesh.ViewMeshToggle.isEnabled(), True)

        mesh.ViewMeshToggle.setChecked(True)
        mesh.ScaleX.setValue(.7)
        mesh.ScaleY.setValue(.9)
        mesh.ScaleZ.setValue(.8)
        mesh.DisplacementToggle.setChecked(False)
        mesh.DisplacementMagnitude.setValue(1.5)
        #self.assertImage('testDiffusion2.png')

        fp._callbackFileList(0)
        self.assertEqual(mesh.isEnabled(), True)
        self.assertEqual(mesh.ScaleX.value(), .9)
        self.assertEqual(mesh.ScaleY.value(), .8)
        self.assertEqual(mesh.ScaleZ.value(), .7)
        self.assertEqual(mesh.DisplacementToggle.isChecked(), True)
        self.assertEqual(mesh.DisplacementMagnitude.value(), 2.0)
        #self.assertImage('testDiffusion1.png')

        fp._callbackFileList(1)
        self.assertEqual(mesh.isEnabled(), True)
        self.assertEqual(mesh.ScaleX.value(), .7)
        self.assertEqual(mesh.ScaleY.value(), .9)
        self.assertEqual(mesh.ScaleZ.value(), .8)
        self.assertEqual(mesh.DisplacementToggle.isChecked(), False)
        self.assertEqual(mesh.DisplacementMagnitude.value(), 1.5)
        #self.assertImage('testDiffusion2.png')

    def testPrefs(self):

        settings = QtCore.QSettings()
        settings.setValue("exodus/defaultColorMap", "magma")
        settings.sync()
        self._widget.cornerWidget().clone.emit()
        self.assertEqual(self._widget.preferencesWidget().count(), 6)
        self.assertEqual(self._widget.currentWidget().ColorbarPlugin.ColorMapList.currentText(), "magma")

        settings.setValue("exodus/defaultColorMap", "default")
        settings.sync()

        self._widget.cornerWidget().clone.emit()
        self.assertEqual(self._widget.currentWidget().ColorbarPlugin.ColorMapList.currentText(), "default")

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
