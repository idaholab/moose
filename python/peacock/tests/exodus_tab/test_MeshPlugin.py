#!/usr/bin/env python
import sys
import unittest
import vtk
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
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.initialize([self._filename])
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)

    def testInitial(self):
        """
        Test the initial state of the widget.
        """
        self.assertTrue(self._widget.MeshPlugin.DisplacementToggle.isChecked())
        self.assertEqual(self._widget.MeshPlugin.DisplacmentMagnitude.value(), 1.0)
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
        self._widget.MeshPlugin.ViewMeshToggle.clicked.emit(True)
        self._window._window.resetCamera() # needed since vtk7.0.0
        self.assertImage('testViewMeshToggle.png', allowed=0.98) # lines are slightly different across platforms

        # Test that toggling representation disable/enables mesh view
        self._widget.MeshPlugin.Representation.setCurrentIndex(1)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isEnabled())

        # Return the index back, it should be re-enabled
        self._widget.MeshPlugin.Representation.setCurrentIndex(0)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(0)
        self.assertTrue(self._widget.MeshPlugin.ViewMeshToggle.isChecked())
        self.assertTrue(self._widget.MeshPlugin.ViewMeshToggle.isEnabled())
        self.assertImage('testViewMeshToggle.png', allowed=0.95) # lines are slightly different across platforms

    def testRepresentation(self):
        """
        Test that the various representation toggles are working.
        """

        # Wirefreme
        self._widget.MeshPlugin.Representation.setCurrentIndex(1)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(1)
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isEnabled())
        self.assertImage('testRepresentationWireframe.png', allowed=0.90) # lines are different across platforms

        # Points
        self._widget.MeshPlugin.Representation.setCurrentIndex(2)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(2)
        self.assertFalse(self._widget.MeshPlugin.ViewMeshToggle.isEnabled())
        self.assertImage('testRepresentationPoints.png', allowed=0.91)

        # Surface
        self._widget.MeshPlugin.Representation.setCurrentIndex(0)
        self._widget.MeshPlugin.Representation.currentIndexChanged.emit(0)
        self.assertTrue(self._widget.MeshPlugin.ViewMeshToggle.isEnabled())
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

    def testDisplacements(self):
        """
        Test displacement toggle and magnitude.
        """

        # Change the file to something with displacements
        filename = '../../chigger/input/displace.e'
        self._window.onFileChanged(filename)
        self._window._reader.update(timestep=4)
        self._window._result.update(camera=None, colorbar={'visible':False}, block=['2'])
        self._window.onResultOptionsChanged({'variable':'distance'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testDisplacmentInitial.png')

        # Set the magnitude
        self._widget.MeshPlugin.DisplacmentMagnitude.setValue(3.1)
        self._widget.MeshPlugin.DisplacmentMagnitude.valueChanged.emit(3.1)
        self.assertImage('testDisplacmentMagnitude.png')

        # Disable displacement
        self._widget.MeshPlugin.DisplacementToggle.setCheckState(QtCore.Qt.Unchecked)
        self._widget.MeshPlugin.DisplacementToggle.clicked.emit(QtCore.Qt.Unchecked)
        self.assertFalse(self._widget.MeshPlugin.DisplacmentMagnitude.isEnabled())
        self.assertImage('testDisplacmentOff.png')

    def testVariableState(self):
        """
        Test that the variable state is stored/loaded.
        """

        # State0
        self._widget.MeshPlugin.onVariableChanged('state0')
        self._widget.MeshPlugin.mesh() # something must be done for state to be stored
        self.assertImage('testInitial.png')

        # State1
        self._widget.MeshPlugin.onVariableChanged('state1')
        self._widget.MeshPlugin.ScaleX.setValue(1.5)
        self._widget.MeshPlugin.ScaleX.valueChanged.emit(1.5)
        self._widget.MeshPlugin.ViewMeshToggle.setCheckState(QtCore.Qt.Checked)
        self._widget.MeshPlugin.ViewMeshToggle.clicked.emit(True)
        self.assertImage('testState1.png', allowed=0.94) # lines are different across platforms

        # Return to state0
        self._widget.MeshPlugin.onVariableChanged('state0')
        self.assertImage('testInitial.png')

        # Return to state1
        self._widget.MeshPlugin.onVariableChanged('state1')
        self.assertImage('testState1.png', allowed=0.94) # lines are different across platforms

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
