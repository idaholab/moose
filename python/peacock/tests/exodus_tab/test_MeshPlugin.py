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
from peacock.ExodusViewer.plugins.MeshPlugin import main
from peacock.utils import Testing

class TestMeshPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to BlockControls widget.
        """

        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window.onCameraChanged((-0.7786, 0.2277, 0.5847), (9.2960, -0.4218, 12.6685), (0.0000, 0.0000, 0.1250))

    def testInitial(self):
        """
        Test the initial state of the widget.
        """
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacementMagnitude.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.Representation.currentText(), 'Surface')
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertEqual(self._widget.MeshPlugin.ScaleX.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleY.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleZ.value(), 1.0)
        self.assertImage('testInitial.png')

    def testViewMeshToggle(self):
        """
        Test that mesh view button is working.
        """

        # Toggle the mesh
        self._widget.MeshPlugin.ViewMeshToggle.setCheckState(QtCore.Qt.Checked)
        self._window._window.resetCamera() # needed since vtk7.0.0
        self.assertImage('testViewMeshToggle.png', allowed=0.97) # lines are slightly different across platforms

        # Test that toggling representation disable/enables mesh view
        self._widget.MeshPlugin.Representation.setCurrentIndex(1)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(1)

        # Return the index back, it should be re-enabled
        self._widget.MeshPlugin.Representation.setCurrentIndex(0)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(0)
        self.assertImage('testViewMeshToggle.png', allowed=0.95) # lines are slightly different across platforms

    def testRepresentation(self):
        """
        Test that the various representation toggles are working.
        """

        # Wirefreme
        self._widget.MeshPlugin.Representation.setCurrentIndex(1)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(1)
        self.assertImage('testRepresentationWireframe.png', allowed=0.90) # lines are different across platforms

        # Points
        self._widget.MeshPlugin.Representation.setCurrentIndex(2)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(2)
        self.assertImage('testRepresentationPoints.png', allowed=0.91)

        # Surface
        self._widget.MeshPlugin.Representation.setCurrentIndex(0)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(0)
        self.assertImage('testInitial.png') # same as initial load

    def testScale(self):
        """
        Test the scaling.
        """

        self._widget.MeshPlugin.ScaleX.setValue(1.5)
        self._widget.MeshPlugin.ScaleX.valueChanged.emit(1.5)
        self.assertImage('testScaleX.png')

        self._widget.MeshPlugin.ScaleY.setValue(1.5)
        self._widget.MeshPlugin.ScaleY.valueChanged.emit(1.5)
        self.assertImage('testScaleY.png')

        self._widget.MeshPlugin.ScaleZ.setValue(0.5)
        self._widget.MeshPlugin.ScaleZ.valueChanged.emit(0.5)
        self.assertImage('testScaleZ.png')

    def testExtents(self):
        """
        Test the extents toggle.
        """
        self._widget.MeshPlugin.Extents.setChecked(True)
        self.assertImage('testExtents.png')
        self._widget.MeshPlugin.Extents.setChecked(False)
        self.assertImage('testInitial.png')

    def testDisplacements(self):
        """
        Test displacement toggle and magnitude.
        """

        # Change the file to something with displacements
        self._widget.FilePlugin.FileList.setCurrentIndex(2)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(2)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2) # distance
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self._window._reader.update(timestep=4)
        self._window._result.update(camera=None, colorbar={'visible':False}, block=['2'])
        self._window.onWindowRequiresUpdate()
        self.assertImage('testDisplacementInitial.png')

        # Set the magnitude
        self._widget.MeshPlugin.DisplacementMagnitude.setValue(3.1)
        self._widget.MeshPlugin.DisplacementMagnitude.valueChanged.emit(3.1)
        self.assertImage('testDisplacementMagnitude.png')

        # Disable displacement
        self._widget.MeshPlugin.DisplacementToggle.setCheckState(QtCore.Qt.Unchecked)
        self.assertFalse(self._widget.MeshPlugin.DisplacementMagnitude.isEnabled())
        self.assertImage('testDisplacementOff.png')

    def testState(self):
        """
        Test that the variable state is stored/loaded.
        """

        # State0
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.DisplacementMagnitude.isEnabled())
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertFalse(self._widget.MeshPlugin.Extents.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacementMagnitude.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.Representation.currentText(), 'Surface')
        self.assertEqual(self._widget.MeshPlugin.ScaleX.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleY.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleZ.value(), 1.0)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'diffused')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentIndex(), 2)
        self.assertImage('testInitial.png')

        # Change state
        self._widget.MeshPlugin.Extents.setChecked(True)
        self._widget.MeshPlugin.Representation.setCurrentIndex(1)
        self._widget.MeshPlugin.ScaleX.setValue(1.1)

        # Assert changes
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'diffused')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentIndex(), 2)
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.DisplacementMagnitude.isEnabled())
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.Extents.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacementMagnitude.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.Representation.currentText(), 'Wireframe')
        self.assertEqual(self._widget.MeshPlugin.ScaleX.value(), 1.1)
        self.assertEqual(self._widget.MeshPlugin.ScaleY.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleZ.value(), 1.0)

        # Change to different variable and change scale
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'convected')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentIndex(), 1)
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.DisplacementMagnitude.isEnabled())
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertFalse(self._widget.MeshPlugin.Extents.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacementMagnitude.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.Representation.currentText(), 'Surface')
        self.assertEqual(self._widget.MeshPlugin.ScaleX.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleY.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleZ.value(), 1.0)

        self._widget.MeshPlugin.ScaleX.setValue(2.1)
        self._widget.MeshPlugin.ScaleX.valueChanged.emit(2.1)

        # Change back
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.FilePlugin.VariableList.currentText(), 'diffused')
        self.assertEqual(self._widget.FilePlugin.VariableList.currentIndex(), 2)
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.DisplacementMagnitude.isEnabled())
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.Extents.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacementMagnitude.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.Representation.currentText(), 'Wireframe')
        self.assertEqual(self._widget.MeshPlugin.ScaleX.value(), 1.1)
        self.assertEqual(self._widget.MeshPlugin.ScaleY.value(), 1.0)
        self.assertEqual(self._widget.MeshPlugin.ScaleZ.value(), 1.0)

    def testStateExtents(self):
        """
        Test that the extents loading actually show up.
        """
        self.assertFalse(self._widget.MeshPlugin.Extents.isChecked())
        self.assertImage('testInitial.png')

        self._widget.MeshPlugin.Extents.setChecked(True)
        self._widget.MeshPlugin.Extents.stateChanged.emit(True)
        self.assertTrue(self._widget.MeshPlugin.Extents.isChecked())
        self.assertImage('testExtentsOn.png')

        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.MeshPlugin.Extents.isChecked())
        self.assertImage('testExtentsOff.png')

        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertTrue(self._widget.MeshPlugin.Extents.isChecked())
        self.assertImage('testExtentsOn.png')

class TestMeshPlugin2(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to BlockControls widget.
        """

        # The file to open
        self._filenames = Testing.get_chigger_input_list('diffusion_1.e', 'diffusion_2.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._window.onWindowRequiresUpdate()

    def testState(self):
        mesh = self._widget.MeshPlugin
        file = self._widget.FilePlugin

        mesh.ScaleX.setValue(0.5)
        mesh.ScaleX.valueChanged.emit(0.5)
        self.assertImage('testDiffusion1.png')

        file.FileList.setCurrentIndex(1)
        file.FileList.currentIndexChanged.emit(1)
        self.assertImage('testDiffusion2.png')

        file.FileList.setCurrentIndex(0)
        file.FileList.currentIndexChanged.emit(0)
        self.assertImage('testDiffusion1.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
