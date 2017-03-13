#!/usr/bin/env python
from peacock.utils import Testing
from PyQt5 import QtCore
from peacock.Input.ParameterInfo import ParameterInfo

class TestAddVariableAndBlock(Testing.PeacockAppImageTestCase):
    """
    Tests that if variables are added that GUI maintains it selections
    """
    def testAdd(self):
        """
        Test that changing the input file resets the VTK window correctly.
        """

        # The tabs to switch between
        input_ = self._app.main_widget.tab_plugin.InputFileEditorWithMesh
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # The variable plugin
        var_plugin = exodus.currentWidget().VariablePlugin
        blk_selector = exodus.currentWidget().BlockPlugin.BlockSelector

        # Run and check output
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['u'])
        self.assertEqual([blk_selector.ListWidget.item(i).text() for i in range(blk_selector.ListWidget.count())], ['0'])

        # Change the colormap (to test that the colormap is maintained)
        idx = [var_plugin.ColorMapList.itemText(i) for i in range(var_plugin.ColorMapList.count())].index('magma')
        var_plugin.ColorMapList.setCurrentIndex(idx)
        var_plugin.ColorMapList.currentIndexChanged.emit(idx)

        # Select the 0 block (to test that the block section is maintained)
        blk_selector.setCheckState([QtCore.Qt.Checked])
        blk_selector.ListWidget.itemClicked.emit(blk_selector.ListWidget.item(0))
        self.assertEqual(blk_selector.ListWidget.item(0).checkState(), QtCore.Qt.Checked)
        self.assertEqual(blk_selector.checkState(), [QtCore.Qt.Checked])
        self.assertEqual(blk_selector.ListHeader.checkState(), QtCore.Qt.Unchecked)

        # Add a variable
        self.selectTab(input_)
        b0 = input_.InputFileEditorPlugin.tree.getBlockInfo("/AuxVariables")
        b0.included = True
        input_.InputFileEditorPlugin.block_tree.copyBlock(b0)
        input_.InputFileEditorPlugin.blockChanged.emit(b0, input_.InputFileEditorPlugin.tree)

        # Add MeshModifier
        b1 = input_.InputFileEditorPlugin.tree.getBlockInfo("/MeshModifiers")
        b1.included = True
        input_.InputFileEditorPlugin.block_tree.copyBlock(b1)
        b2 = b1.children.values()[0]
        b2.setBlockType('SubdomainBoundingBox')
        b2.addParameter(ParameterInfo(None, 'bottom_left'))
        b2.setParamValue('bottom_left', '0.25 0.25 0')
        b2.addParameter(ParameterInfo(None, 'top_right'))
        b2.setParamValue('top_right', '0.75 0.75 0')
        b2.addParameter(ParameterInfo(None, 'block_id'))
        b2.setParamValue('block_id', '1980')

        input_.InputFileEditorPlugin.blockChanged.emit(b1, input_.InputFileEditorPlugin.tree)

        # Run and check output
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['New_0', 'u'])

        # Check colormap
        self.assertEqual(self._window._result.getOption('cmap'), 'magma')
        self.assertEqual(var_plugin.ColorMapList.currentText(), 'magma')

        # Check variable
        self.assertEqual(self._window._result.getOption('variable'), 'u')
        self.assertEqual(var_plugin.VariableList.currentText(), 'u')

        # Check blocks
        self.assertEqual([blk_selector.ListWidget.item(i).text() for i in range(blk_selector.ListWidget.count())], ['0', '1980'])
        self.assertEqual(blk_selector.ListWidget.item(0).checkState(), QtCore.Qt.Checked)
        self.assertEqual(blk_selector.ListWidget.item(1).checkState(), QtCore.Qt.Unchecked)
        self.assertEqual(blk_selector.checkState(), [QtCore.Qt.Checked, QtCore.Qt.Unchecked])


if __name__ == '__main__':
    Testing.run_tests()
