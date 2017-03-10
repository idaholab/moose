#!/usr/bin/env python
import sys
import unittest
import vtk
from PyQt5 import QtWidgets

from peacock.ExodusViewer.ExodusViewer import main
from peacock.utils import Testing


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
        self._widget = main(size=[400,400])
        self._widget.initialize([self._filename])

        # Start with 'diffused' variable
        self._widget.currentWidget().VariablePlugin.VariableList.setCurrentIndex(2)
        self._widget.currentWidget().VariablePlugin.VariableList.currentIndexChanged.emit(2)


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
            self.assertImage('testInitial.png')
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')

    def testCloneClose(self):
        """
        Test clone button works.
        """
        self._widget.cornerWidget().clone.emit()
        self._widget.currentWidget().VariablePlugin.VariableList.setCurrentIndex(2)
        self._widget.currentWidget().VariablePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.count(), 2)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertTrue(self._widget.cornerWidget().CloseButton.isEnabled())

        if sys.platform == 'darwin':
            self.assertImage('testInitial.png')

        # Change camera on cloned tab
        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._widget.currentWidget().VTKWindowPlugin.onCameraChanged(camera)
        if sys.platform == 'darwin':
            self.assertImage('testClone.png')

        # Switch to first tab
        self._widget.setCurrentIndex(0)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results')
        if sys.platform == 'darwin':
            self.assertImage('testInitial.png')

        # Close the first tab
        self._widget.cornerWidget().close.emit()
        self.assertEqual(self._widget.count(), 1)
        self.assertEqual(self._widget.tabText(self._widget.currentIndex()), 'Results (2)')
        self.assertFalse(self._widget.cornerWidget().CloseButton.isEnabled())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
