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
        self._filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
        self._widget, self._window = main(size=[600,600])
        self._widget.FilePlugin.onSetFilenames(self._filenames)
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)

        camera = vtk.vtkCamera()
        camera.SetViewUp(-0.7786, 0.2277, 0.5847)
        camera.SetPosition(9.2960, -0.4218, 12.6685)
        camera.SetFocalPoint(0.0000, 0.0000, 0.1250)
        self._window.onCameraChanged(camera.GetViewUp(), camera.GetPosition(), camera.GetFocalPoint())
        self._window.onWindowRequiresUpdate()

    def testBlocks(self):
        """
        Test the block selection.
        """
        # By default all blocks should be selected
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])

        # Uncheck a block
        item =  self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(2)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1'])
        self.assertImage('testBlocks.png')

        # Uncheck "all"
        item =  self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), None)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        item =  self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])
        self.assertImage('testBlocksAll.png')

    def testSidesets(self):
        """
        Test the sidesets selection.
        """

        # By default no sidesets should be selected
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Uncheck block and select "all" the sidesets
        item =  self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), None)

        item =  self._widget.BlockPlugin.SidesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), sorted(['top', 'bottom']))
        self.assertImage('testSidesetsAll.png')

        # Uncheck a sideset
        item =  self._widget.BlockPlugin.SidesetSelector.StandardItemModel.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.SidesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), ['top'])
        self.assertImage('testSidesets.png')

        # Uncheck "all"
        item =  self._widget.BlockPlugin.SidesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.SidesetSelector.StandardItemModel.itemChanged.emit(item)

        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        item =  self._widget.BlockPlugin.SidesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), sorted(['top', 'bottom']))
        self.assertImage('testSidesetsAll.png')

    def testNodesets(self):
        """
        Test the nodesets selection.
        """
        # By default no nodesets should be selected
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)

        # Uncheck block and select "all" the nodesets
        item =  self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), None)

        item =  self._widget.BlockPlugin.NodesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.NodesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['1', '2'])
        self.assertImage('testNodesetsAll.png', allowed=0.97)

        # Uncheck a nodeet
        item =  self._widget.BlockPlugin.NodesetSelector.StandardItemModel.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.NodesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['2'])
        self.assertImage('testNodesets.png')

        # Uncheck "all"
        item =  self._widget.BlockPlugin.NodesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.NodesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertImage('testBlocksEmpty.png')

        # Check "all"
        item =  self._widget.BlockPlugin.NodesetSelector.StandardItemModel.item(0)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.NodesetSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), ['1', '2'])
        self.assertImage('testNodesetsAll.png', allowed=0.97)

    def testState(self):
        """
        Test that state is stored with variable changes.
        """
        # Initial state
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Disable a block
        item = self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)

        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Change to 'convected' and check that all blocks are selected
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Disable a block and select a sideset
        item = self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(2)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)

        item = self._widget.BlockPlugin.SidesetSelector.StandardItemModel.item(2)
        item.setCheckState(QtCore.Qt.Checked)
        self._widget.BlockPlugin.SidesetSelector.StandardItemModel.itemChanged.emit(item)

        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1'])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), ['top'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)

        # Go back to first item
        self._widget.FilePlugin.VariableList.setCurrentIndex(2)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(2)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Back to convected
        self._widget.FilePlugin.VariableList.setCurrentIndex(1)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1'])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), ['top'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)

    def testState2(self):
        """
        Test state change with changing filename.
        """
        # Initial state
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['1', '76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Disable a block
        item = self._widget.BlockPlugin.BlockSelector.StandardItemModel.item(1)
        item.setCheckState(QtCore.Qt.Unchecked)
        self._widget.BlockPlugin.BlockSelector.StandardItemModel.itemChanged.emit(item)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

        # Change files
        self._widget.FilePlugin.FileList.setCurrentIndex(1)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(1)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['0'])
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)

        # Change files back
        self._widget.FilePlugin.FileList.setCurrentIndex(0)
        self._widget.FilePlugin.FileList.currentIndexChanged.emit(0)
        self.assertEqual(self._widget.BlockPlugin.BlockSelector.getBlocks(), ['76'])
        self.assertEqual(self._widget.BlockPlugin.NodesetSelector.getBlocks(), None)
        self.assertEqual(self._widget.BlockPlugin.SidesetSelector.getBlocks(), None)

    def testElementalVariable(self):
        """
        Test that elemental variables disable boundary/nodeset
        """
        self.assertTrue(self._widget.BlockPlugin.BlockSelector.isEnabled())
        self.assertTrue(self._widget.BlockPlugin.SidesetSelector.isEnabled())
        self.assertTrue(self._widget.BlockPlugin.NodesetSelector.isEnabled())

        self._widget.FilePlugin.VariableList.setCurrentIndex(0)
        self._widget.FilePlugin.VariableList.currentIndexChanged.emit(0)

        self.assertTrue(self._widget.BlockPlugin.BlockSelector.isEnabled())
        self.assertFalse(self._widget.BlockPlugin.SidesetSelector.isEnabled())
        self.assertFalse(self._widget.BlockPlugin.NodesetSelector.isEnabled())

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
