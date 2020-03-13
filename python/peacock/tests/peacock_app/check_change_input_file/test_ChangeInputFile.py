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
from PyQt5 import QtWidgets
import os

class TestChangeInputFile(Testing.PeacockAppImageTestCase):
    """
    Tests that if the input file changes the VTKWindow is reset.
    """
    qapp = QtWidgets.QApplication([])

    def testInputReset(self):
        """
        Test that changing the input file resets the VTK window correctly.
        """

        # The tabs to switch between
        start_dir = os.getcwd()
        input_ = self._app.main_widget.tab_plugin.InputFileEditorWithMesh
        exodus = self._app.main_widget.tab_plugin.ExodusViewer
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin

        # The variable plugin
        var_plugin = exodus.currentWidget().FilePlugin
        blk_selector = exodus.currentWidget().BlockPlugin.BlockSelector
        cmap_plugin = exodus.currentWidget().ColorbarPlugin

        # Run and check output
        self.selectTab(execute)
        execute.ExecuteOptionsPlugin.setWorkingDir(start_dir)
        Testing.process_events(t=2)
        self.execute()
        self.selectTab(exodus)
        Testing.process_events(t=2)

        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['u'])
        self.assertEqual([blk_selector.StandardItemModel.item(i).text() for i in range(1, blk_selector.StandardItemModel.rowCount())], ['0'])

        # Change the colormap (to test that the colormap is maintained)
        idx = [cmap_plugin.ColorMapList.itemText(i) for i in range(cmap_plugin.ColorMapList.count())].index('magma')
        cmap_plugin.ColorMapList.setCurrentIndex(idx)
        cmap_plugin.ColorMapList.currentIndexChanged.emit(idx)

        # Change the input
        self.selectTab(input_)
        input_.setInputFile("../../common/simple_diffusion2.i")

        # Run and check output
        self.selectTab(execute)
        execute.ExecuteOptionsPlugin.setWorkingDir(start_dir)
        self.execute()
        self.selectTab(exodus)
        self.assertEqual([var_plugin.VariableList.itemText(i) for i in range(var_plugin.VariableList.count())], ['aux', 'not_u', 'u'])
        self.assertEqual([blk_selector.StandardItemModel.item(i).text() for i in range(1, blk_selector.StandardItemModel.rowCount())], ['0', '1980'])

        # Check colormap
        self.assertEqual(self._window._result.getOption('cmap'), 'default')
        self.assertEqual(cmap_plugin.ColorMapList.currentText(), 'default')

        # Check variable
        self.assertEqual(self._window._result.getOption('variable'), 'aux')
        self.assertEqual(var_plugin.VariableList.currentText(), 'aux')

if __name__ == '__main__':
    Testing.run_tests()
