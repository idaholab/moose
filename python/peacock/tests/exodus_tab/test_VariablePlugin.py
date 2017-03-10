#!/usr/bin/env python
import sys
import unittest
from PyQt5 import QtCore, QtWidgets
from peacock.ExodusViewer.plugins.VariablePlugin import main
from peacock.utils import Testing

class TestVariablePlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """

        # The file to open
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
        print self._filenames
        self._widget, self._window = main(size=[400,400])
        self._widget.initialize(self._filenames)

        # Start with 'diffused' variable
        self._widget.VariablePlugin.VariableList.setCurrentIndex(2)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(2)


    def testInitial(self):
        """
        Test the initial state of the widget.
        """
        self.assertEqual(self._widget.VariablePlugin.VariableList.currentText(), 'diffused')
        self.assertEqual(self._widget.VariablePlugin.ComponentList.currentText(), 'Magnitude')
        self.assertFalse(self._widget.VariablePlugin.ComponentList.isEnabled())
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.0')
        self.assertTrue(self._widget.VariablePlugin.RangeMinimum.isEnabled())
        self.assertIn('color:#8C8C8C', self._widget.VariablePlugin.RangeMinimum.styleSheet())
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '2.0')
        self.assertTrue(self._widget.VariablePlugin.RangeMaximum.isEnabled())
        self.assertIn('color:#8C8C8C', self._widget.VariablePlugin.RangeMaximum.styleSheet())
        self.assertEqual(self._widget.VariablePlugin.ColorMapList.currentText(), 'default')
        self.assertFalse(self._widget.VariablePlugin.ReverseColorMap.isChecked())
        self.assertTrue(self._widget.VariablePlugin.ColorBarToggle.isChecked())
        self.assertImage('testInitial.png')

    def testVariable(self):
        """
        Test changing variables.
        """
        self._widget.VariablePlugin.VariableList.setCurrentIndex(0)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(0)
        self.assertImage('testVariable.png')

    def testVector(self):
        """
        Test changing vector stuff.
        """
        # Change the file to something with vectors
        self._window.onFileChanged(self._filenames[1])
        self._widget.VariablePlugin.VariableList.setCurrentIndex(1)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertImage('testVectorInitial.png')

        # Change to x
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(1)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(1)
        self.assertImage('testVectorX.png')

        # Change to y
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(2)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(2)
        self.assertImage('testVectorY.png')

    def testVectorState(self):
        """
        Test changing component load/stores range state.
        """

        # Change the file to something with vectors
        self._window.onFileChanged(self._filenames[1])
        self._widget.VariablePlugin.VariableList.setCurrentIndex(1)
        self._widget.VariablePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.0')
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '2.2360679775')

        # Change state on magnitude
        self._widget.VariablePlugin.RangeMinimum.setText('0.0')
        self._widget.VariablePlugin.RangeMaximum.setText('1.2345')
        self._widget.VariablePlugin.RangeMaximum.editingFinished.emit()

        # Change to x component and change gui state
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(1)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(1)
        self._widget.VariablePlugin.RangeMinimum.setText('0.12345')
        self._widget.VariablePlugin.RangeMinimum.editingFinished.emit()

        # Change back to magnitude and test state
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(0)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.0')
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '1.2345')

        # Change back to x-component and test state
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(1)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.12345')

        # Unset maximum on x-component state
        self._widget.VariablePlugin.RangeMaximum.setText('')
        self._widget.VariablePlugin.RangeMaximum.editingFinished.emit()
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '1.0')

        # Check magnitude state again
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(0)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '1.2345')
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.0')

        # Check x-component state again
        self._widget.VariablePlugin.ComponentList.setCurrentIndex(1)
        self._widget.VariablePlugin.ComponentList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '1.0')
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.12345')

    def testRange(self):
        """
        Test that custom range is operational.
        """

        # Minimum
        self.assertTrue(self._widget.VariablePlugin.RangeMinimum.isEnabled())
        self._widget.VariablePlugin.RangeMinimum.setText(str(0.15))
        self._widget.VariablePlugin.RangeMinimum.editingFinished.emit()
        self.assertIn('color:#8C8C8C', self._widget.VariablePlugin.RangeMaximum.styleSheet())
        self.assertImage('testRangeMinimum.png')

        # Maximum
        self._widget.VariablePlugin.RangeMinimum.setText('')
        self._widget.VariablePlugin.RangeMinimum.editingFinished.emit()
        self._widget.VariablePlugin.RangeMaximum.setText(str(1.5))
        self._widget.VariablePlugin.RangeMaximum.editingFinished.emit()
        self.assertIn('color:#8C8C8C', self._widget.VariablePlugin.RangeMinimum.styleSheet())
        self.assertImage('testRangeMaximum.png')

        # Min/Max
        self._widget.VariablePlugin.RangeMinimum.setText(str(0.5))
        self._widget.VariablePlugin.RangeMinimum.editingFinished.emit()
        self._widget.VariablePlugin.RangeMaximum.setText(str(1.5))
        self._widget.VariablePlugin.RangeMaximum.editingFinished.emit()
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.5')
        self.assertEqual(self._widget.VariablePlugin.RangeMaximum.text(), '1.5')
        self.assertImage('testRangeMinMaximum.png')

    def testColormap(self):
        """
        Test that colormap can be changed and reversed.
        """

        index = self._widget.VariablePlugin.ColorMapList.findText('viridis')
        self._widget.VariablePlugin.ColorMapList.setCurrentIndex(index)
        self._widget.VariablePlugin.ColorMapList.currentIndexChanged.emit(index)
        self.assertImage('testColorMap.png')

        # Reverse
        self._widget.VariablePlugin.ReverseColorMap.setCheckState(QtCore.Qt.Checked)
        self._widget.VariablePlugin.ReverseColorMap.clicked.emit(QtCore.Qt.Checked)
        self.assertImage('testColorMapReversed.png')

        # Test going back works
        self._widget.VariablePlugin.ReverseColorMap.setCheckState(QtCore.Qt.Unchecked)
        self._widget.VariablePlugin.ReverseColorMap.clicked.emit(QtCore.Qt.Unchecked)
        self.assertImage('testColorMap.png')

    def testColorBar(self):
        """
        Test that the colorbar can be disabled.
        """
        self.assertImage('testInitial.png')
        self._widget.VariablePlugin.ColorBarToggle.setCheckState(QtCore.Qt.Unchecked)
        self._widget.VariablePlugin.ColorBarToggle.clicked.emit(QtCore.Qt.Unchecked)
        self.assertImage('testColorBarOff.png')
        self._widget.VariablePlugin.ColorBarToggle.setCheckState(QtCore.Qt.Checked)
        self._widget.VariablePlugin.ColorBarToggle.clicked.emit(QtCore.Qt.Checked)
        self.assertImage('testInitial.png')

    def testAutoRangeUpdate(self):
        """
        Test that the range updates when the time changes.
        """
        self._widget.VTKWindowPlugin._reader.update(timestep=0)
        self._widget.VTKWindowPlugin.onWindowRequiresUpdate()
        self._widget.VariablePlugin.onTimeChanged()
        self.assertEqual(self._widget.VariablePlugin.RangeMinimum.text(), '0.0')
        self.assertAlmostEqual(float(self._widget.VariablePlugin.RangeMaximum.text()), 0.0)
        self.assertIn('color:#8C8C8C', self._widget.VariablePlugin.RangeMinimum.styleSheet())


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
