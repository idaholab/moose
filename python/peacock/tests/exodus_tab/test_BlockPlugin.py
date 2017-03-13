#!/usr/bin/env python
import sys
import unittest
import vtk
from PyQt5 import QtCore, QtWidgets
from peacock.ExodusViewer.plugins.BlockPlugin import main
from peacock.utils import Testing

class TestBlockPlugin(Testing.PeacockImageTestCase):
    """
    Testing for BlockControl widget.
    """

    qapp = QtWidgets.QApplication(sys.argv)

    def setUp(self):
        """
        Creates a window attached to BlockControls widget.
        """

        # The file to open
        self._filename = Testing.get_chigger_input('mug_blocks_out.e')
        self._widget, self._window = main(size=[600,600])
        self._window.onFileChanged(self._filename)

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera)

    def testBlocks(self):
        """
        Test the block selection.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # By default all blocks should be selected
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertTrue(self._widget.BlockPlugin.BlockSelector.ListHeader.isChecked())

        # Uncheck a block
        item =  self._widget.BlockPlugin.BlockSelector.ListWidget.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Unchecked])
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.ListHeader.checkState(), QtCore.Qt.PartiallyChecked)
        self.assertImage('testBlocks.png')

        # Uncheck "all"
        self._widget.BlockPlugin.BlockSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Unchecked])
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.ListHeader.checkState(), QtCore.Qt.Unchecked)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        self._widget.BlockPlugin.BlockSelector.ListHeader.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.BlockSelector.ListHeader.clicked.emit(QtCore.Qt.Checked)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.ListHeader.checkState(), QtCore.Qt.Checked)
        self.assertImage('testBlocksAll.png')

    def testSidesets(self):
        """
        Test the sidesets selection.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # By default no sidesets should be selected
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Unchecked])
        self.assertFalse(self._widget.BlockPlugin.SidesetSelector.ListHeader.isChecked())

        # Uncheck block and select "all" the sidesets
        self._widget.BlockPlugin.BlockSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), None)
        self._widget.BlockPlugin.SidesetSelector.ListHeader.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.ListHeader.clicked.emit(QtCore.Qt.Checked)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), sorted(['top', 'bottom']))
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.ListHeader.checkState(), QtCore.Qt.Checked)
        self.assertImage('testSidesetsAll.png')

        # Uncheck a sideset
        item =  self._widget.BlockPlugin.SidesetSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.SidesetSelector.ListWidget.itemClicked.emit(item)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), ['top'])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.ListHeader.checkState(), QtCore.Qt.PartiallyChecked)
        self.assertImage('testSidesets.png')

        # Uncheck "all"
        self._widget.BlockPlugin.SidesetSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.SidesetSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Unchecked])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.ListHeader.checkState(), QtCore.Qt.Unchecked)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        self._widget.BlockPlugin.SidesetSelector.ListHeader.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.ListHeader.clicked.emit(QtCore.Qt.Checked)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), sorted(['top', 'bottom']))
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.ListHeader.checkState(), QtCore.Qt.Checked)
        self.assertImage('testSidesetsAll.png')

    def testNodesets(self):
        """
        Test the nodesets selection.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # By default no nodesets should be selected
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Unchecked])
        self.assertFalse(self._widget.BlockPlugin.NodesetSelector.ListHeader.isChecked())

        # Uncheck block and select "all" the nodesets
        self._widget.BlockPlugin.BlockSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), None)
        self._widget.BlockPlugin.NodesetSelector.ListHeader.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.NodesetSelector.ListHeader.clicked.emit(QtCore.Qt.Checked)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['1', '2'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.ListHeader.checkState(), QtCore.Qt.Checked)
        self.assertImage('testNodesetsAll.png', allowed=0.97)

        # Uncheck a nodeet
        item =  self._widget.BlockPlugin.NodesetSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.NodesetSelector.ListWidget.itemClicked.emit(item)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['2'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.ListHeader.checkState(), QtCore.Qt.PartiallyChecked)
        self.assertImage('testNodesets.png')

        # Uncheck "all"
        self._widget.BlockPlugin.NodesetSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.NodesetSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.checkState(), [QtCore.Qt.Unchecked, QtCore.Qt.Unchecked])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.ListHeader.checkState(), QtCore.Qt.Unchecked)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        self._widget.BlockPlugin.NodesetSelector.ListHeader.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.NodesetSelector.ListHeader.clicked.emit(QtCore.Qt.Checked)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['1', '2'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Checked])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.ListHeader.checkState(), QtCore.Qt.Checked)
        self.assertImage('testNodesetsAll.png', allowed=0.97)

    def testContour(self):
        """
        Test that contour flag disables nodeset/sidesets.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # Show a single block and single sideset
        item = self._widget.BlockPlugin.BlockSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)

        item = self._widget.BlockPlugin.SidesetSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.ListWidget.itemClicked.emit(item)

        self.assertImage('testContourInitial.png')

        # Select the contour button (removes sidesets)
        self._widget.BlockPlugin.onContourClicked(True)
        self.assertImage('testContour.png')

    def testContourState(self):
        """
        Test that contour flag saves/loads state.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # Show a single block and single sideset
        item = self._widget.BlockPlugin.BlockSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)

        item = self._widget.BlockPlugin.SidesetSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.ListWidget.itemClicked.emit(item)

        self.assertImage('testContourInitial.png')

        # Toggle contour and change active block
        self._widget.BlockPlugin.onContourClicked(True)
        item = self._widget.BlockPlugin.BlockSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)
        item = self._widget.BlockPlugin.BlockSelector.ListWidget.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)
        self.assertImage('testContourState.png')

        # Unset contour; the original block/boundary state should be same as initial
        self._widget.BlockPlugin.onContourClicked(False)
        self.assertImage('testContourInitial.png')

        # Reset contour; should go back to state1
        self._widget.BlockPlugin.onContourClicked(True)
        self.assertImage('testContourState.png')

    def testVariableState(self):
        """
        Test that state is stored with variable changes.
        """
        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        # state0: Show a single block and single sideset
        self._widget.BlockPlugin.onVariableChanged('state0')
        item = self._widget.BlockPlugin.BlockSelector.ListWidget.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListWidget.itemClicked.emit(item)

        item = self._widget.BlockPlugin.SidesetSelector.ListWidget.item(1)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.ListWidget.itemClicked.emit(item)

        self.assertImage('testContourState0.png')

        # State1: Disable blocks
        self._widget.BlockPlugin.onVariableChanged('state1')
        self._widget.BlockPlugin.BlockSelector.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.ListHeader.clicked.emit(QtCore.Qt.Unchecked)
        self.assertImage('testVariableState1.png')

        # Return to state0:
        self._widget.BlockPlugin.onVariableChanged('state0')
        self.assertImage('testContourState0.png')

        # Return to state1
        self._widget.BlockPlugin.onVariableChanged('state1')
        self.assertImage('testVariableState1.png')

    def testElementalVariable(self):
        """
        Test that elemental variables disable boundary/nodeset
        """
        self.assertFalse(self._widget.BlockPlugin.SidesetSelector.isEnabled())
        self.assertFalse(self._widget.BlockPlugin.NodesetSelector.isEnabled())

        self._window.onResultOptionsChanged({'variable':'diffused'})
        self._window.onWindowRequiresUpdate()

        self.assertTrue(self._widget.BlockPlugin.SidesetSelector.isEnabled())
        self.assertTrue(self._widget.BlockPlugin.NodesetSelector.isEnabled())



if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
