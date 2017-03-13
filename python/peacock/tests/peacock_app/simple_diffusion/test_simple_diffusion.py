#!/usr/bin/env python
from peacock.utils import Testing, InputTesting

class Tests(Testing.PeacockTester, InputTesting.InputTreeTester):
    def create_app(self, args):
        self.createPeacockApp(args)
        self.result = self.app.main_widget.tab_plugin.ExodusViewer
        self.vtkwin = self.result.currentWidget().VTKWindowPlugin

    def check_current_tab(self, tabs, name):
        self.assertEqual(str(tabs.tabText(tabs.currentIndex())), name)

    def testSimpleDiffusion(self):
        self.create_app([Testing.find_moose_test_exe()])
        input_plugin = self.app.main_widget.tab_plugin.InputFileEditorWithMesh
        self.clickTab(self.app.main_widget, input_plugin.tabName())
        exe_plugin = self.app.main_widget.tab_plugin.ExecuteTabPlugin
        self.createSimpleDiffusion(input_plugin)
        self.clickTab(self.app.main_widget, exe_plugin.tabName())
        run_buttons = Testing.findQObjectsByName(exe_plugin.ExecuteRunnerPlugin, "run_button")
        self.clickButton(run_buttons)
        self.clickTab(self.app.main_widget, self.result.tabName())
        Testing.set_window_size(self.vtkwin)
        fname = "simple_diffusion_results.png"
        Testing.remove_file(fname)
        self.vtkwin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

if __name__ == '__main__':
    Testing.run_tests()
