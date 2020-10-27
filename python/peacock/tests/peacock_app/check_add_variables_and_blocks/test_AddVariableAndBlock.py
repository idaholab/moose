#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import Testing
from PyQt5 import QtCore, QtWidgets
from peacock.Input.ParameterInfo import ParameterInfo

class TestAddVariableAndBlock(Testing.PeacockAppImageTestCase):
    """
    Tests that if variables are added that GUI maintains it selections
    """
    qapp = QtWidgets.QApplication([])

    def testAdd(self):
        """
        Test that changing the input file resets the VTK window correctly.
        """

        # The tabs to switch between
        input_ = self._app.main_widget.tab_plugin.InputFileEditorWithMesh
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # The variable plugin
        var_plugin = exodus.currentWidget().FilePlugin
        blk_selector = exodus.currentWidget().BlockPlugin.BlockSelector
        cmap_plugin = exodus.currentWidget().ColorbarPlugin

        # Run and check output
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        Testing.process_events(t=2)

        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['u'])
        self.assertEqual([blk_selector.StandardItemModel.item(i).text() for i in range(1, blk_selector.StandardItemModel.rowCount())], ['0'])

        # Change the colormap (to test that the colormap is maintained)
        idx = [cmap_plugin.ColorMapList.itemText(i) for i in range(cmap_plugin.ColorMapList.count())].index('magma')
        cmap_plugin.ColorMapList.setCurrentIndex(idx)
        cmap_plugin.ColorMapList.currentIndexChanged.emit(idx)

        # Select the 0 block (to test that the block section is maintained)
        blk_selector.StandardItemModel.item(0).setCheckState(QtCore.Qt.Checked)
        self.assertEqual(blk_selector.StandardItemModel.item(0).checkState(), QtCore.Qt.Checked)
        self.assertEqual(blk_selector.StandardItemModel.item(1).checkState(), QtCore.Qt.Checked)

        # Add a variable
        self.selectTab(input_)
        b0 = input_.InputFileEditorPlugin.tree.getBlockInfo("/AuxVariables")
        b0.included = True
        input_.InputFileEditorPlugin.block_tree.copyBlock(b0)
        input_.InputFileEditorPlugin.blockChanged.emit(b0, input_.InputFileEditorPlugin.tree)

        # Add MeshModifier
        b1 = input_.InputFileEditorPlugin.tree.getBlockInfo("/Mesh")
        b1.included = True
        input_.InputFileEditorPlugin.block_tree.copyBlock(b1)
        b2 = list(b1.children.values())[-1]
        b2.setBlockType('SubdomainBoundingBoxGenerator')
        b2.addParameter(ParameterInfo(None, 'bottom_left'))
        b2.setParamValue('bottom_left', '0.25 0.25 0')
        b2.addParameter(ParameterInfo(None, 'top_right'))
        b2.setParamValue('top_right', '0.75 0.75 0')
        b2.addParameter(ParameterInfo(None, 'block_id'))
        b2.setParamValue('block_id', '1980')
        b2.addParameter(ParameterInfo(None, 'input'))
        b2.setParamValue('input', 'generate')

        input_.InputFileEditorPlugin.blockChanged.emit(b1, input_.InputFileEditorPlugin.tree)

        # Run and check output
        self.selectTab(execute)
        Testing.process_events(t=2)
        self.execute()
        self.selectTab(exodus)

        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['New_0', 'u'])

        # Check colormap
        self.assertEqual(self._window._result.getOption('cmap'), 'default')
        self.assertEqual(cmap_plugin.ColorMapList.currentText(), 'default')

        # Check variable
        self.assertEqual(self._window._result.getOption('variable'), 'New_0')
        self.assertEqual(var_plugin.VariableList.currentText(), 'New_0')

        # Check blocks
        self.assertEqual([blk_selector.StandardItemModel.item(i).text() for i in range(1, blk_selector.StandardItemModel.rowCount())], ['0', '1980'])
        self.assertEqual(blk_selector.StandardItemModel.item(0).checkState(), QtCore.Qt.Checked)
        self.assertEqual(blk_selector.StandardItemModel.item(1).checkState(), QtCore.Qt.Checked)
        self.assertEqual(blk_selector.StandardItemModel.item(2).checkState(), QtCore.Qt.Checked)


if __name__ == '__main__':
    Testing.run_tests()
