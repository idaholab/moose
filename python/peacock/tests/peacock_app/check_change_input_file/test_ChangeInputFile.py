#!/usr/bin/env python
from peacock.utils import Testing

class TestChangeInputFile(Testing.PeacockAppImageTestCase):
    """
    Tests that if the input file changes the VTKWindow is reset.
    """
    def testInputReset(self):
        """
        Test that changing the input file resets the VTK window correctly.
        """

        # The tabs to switch between
        input_ = self._app.main_widget.tab_plugin.InputFileEditorWithMesh
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # The variable plugin
        var_plugin = exodus.currentWidget().VariablePlugin
        blk_selector = exodus.currentWidget().BlockPlugin.BlockSelector.ListWidget

        # Run and check output
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)

        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['u'])
        self.assertEqual([blk_selector.item(i).text() for i in range(blk_selector.count())], ['0'])

        # Change the colormap (to test that the colormap is maintained)
        idx = [var_plugin.ColorMapList.itemText(i) for i in range(var_plugin.ColorMapList.count())].index('magma')
        var_plugin.ColorMapList.setCurrentIndex(idx)
        var_plugin.ColorMapList.currentIndexChanged.emit(idx)

        # Change the input
        self.selectTab(input_)
        input_.setInputFile("../../common/simple_diffusion2.i")

        # Run and check output
        self.selectTab(execute)
        self.execute()
        self.selectTab(exodus)
        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['aux', 'not_u', 'u'])
        self.assertEqual([blk_selector.item(i).text() for i in range(blk_selector.count())], ['0', '1980'])

        # Check colormap
        self.assertEqual(self._window._result.getOption('cmap'), 'magma')
        self.assertEqual(var_plugin.ColorMapList.currentText(), 'magma')

        # Check variable
        self.assertEqual(self._window._result.getOption('variable'), 'u')
        self.assertEqual(var_plugin.VariableList.currentText(), 'u')

if __name__ == '__main__':
    Testing.run_tests()
