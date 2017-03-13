#!/usr/bin/env python
import sys
import unittest
from PyQt5 import QtCore, QtWidgets, QtGui
from peacock.ExodusViewer.plugins.BackgroundPlugin import main
from peacock.utils import Testing

class TestBackgroundPlugin(Testing.PeacockImageTestCase):
    """
    Testing for MeshControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to FilePlugin widget.
        """

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.initialize([self._filename])
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

    def testInitial(self):
        """
        Test the initial state of the widget.
        """

        bottom = self._window._window[0].getVTKRenderer().GetBackground()
        bottom_exact = 0.7058823529411765
        for i in range(3):
            self.assertAlmostEqual(bottom[i], bottom_exact)

        top = self._window._window[0].getVTKRenderer().GetBackground2()
        top_exact = 0.43529411764705883
        for i in range(3):
            self.assertAlmostEqual(top[i], top_exact)

        self.assertImage('testInitial.png')

    def testChangeTop(self):
        """
        Test changing top color.
        """

        self._widget.BackgroundPlugin._top = QtGui.QColor(0,255,0)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testTopColor.png')

    def testChangeBottom(self):
        """
        Test changing bottom color.
        """

        self._widget.BackgroundPlugin._bottom = QtGui.QColor(0,0,255)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testBottomColor.png')

    def testSolidColor(self):
        """
        Test gradient toggle.
        """
        self._widget.BackgroundPlugin.GradientToggle.setChecked(QtCore.Qt.Unchecked)
        self._widget.BackgroundPlugin.GradientToggle.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BackgroundPlugin.TopLabel.text(), 'Background Color:')
        self._widget.BackgroundPlugin._top = QtGui.QColor(255,0,0)
        self._widget.BackgroundPlugin.color()
        self.assertImage('testSolidColor.png')

    def testExtents(self):
        """
        Test the extents toggle.
        """
        self._widget.BackgroundPlugin.Extents.clicked.emit(True)
        self.assertImage('testExtents.png')
        self._widget.BackgroundPlugin.Extents.clicked.emit(False)
        self.assertImage('testInitial.png')

    def testElementLabels(self):
        """
        Test the element label toggle.
        """
        self._widget.BackgroundPlugin.Elements.clicked.emit(True)
        self.assertImage('testElements.png')
        self._widget.BackgroundPlugin.Elements.clicked.emit(False)
        self.assertImage('testInitial.png')

    def testNodeLabels(self):
        """
        Test the node label toggle.
        """
        self._widget.BackgroundPlugin.Nodes.clicked.emit(True)
        self.assertImage('testNodes.png')
        self._widget.BackgroundPlugin.Nodes.clicked.emit(False)
        self.assertImage('testInitial.png')

    def testNodalValueLabels(self):
        """
        Test the label of nodal data toggle.
        """
        self._widget.BackgroundPlugin.Values.clicked.emit(True)
        self.assertImage('testNodalValues.png')
        self._widget.BackgroundPlugin.Values.clicked.emit(False)
        self.assertImage('testInitial.png')

    def testElementalValueLabels(self):
        """
        Test the label of elemental data toggle.
        """
        self._window.onResultOptionsChanged({'variable':'aux_elem'})
        self._window.onWindowRequiresUpdate()
        self.assertImage('testElementalInitial.png')
        self._widget.BackgroundPlugin.Values.clicked.emit(True)
        self.assertImage('testElementalValues.png')
        self._widget.BackgroundPlugin.Values.clicked.emit(False)
        self.assertImage('testElementalInitial.png')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
