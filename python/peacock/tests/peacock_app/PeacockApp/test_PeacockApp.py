#!/usr/bin/env python
from peacock.utils import Testing
import os

class Tests(Testing.PeacockTester):
    def create_app(self, args):
        self.createPeacockApp(args)
        self.postprocessor = self.app.main_widget.tab_plugin.PostprocessorViewer
        self.vector_postprocessor = self.app.main_widget.tab_plugin.VectorPostprocessorViewer
        self.exe = self.app.main_widget.tab_plugin.ExecuteTabPlugin
        self.input = self.app.main_widget.tab_plugin.InputFileEditorWithMesh
        self.result = self.app.main_widget.tab_plugin.ExodusViewer
        self.vtkwin = self.result.currentWidget().VTKWindowPlugin

    def check_current_tab(self, tabs, name):
        self.assertEqual(str(tabs.tabText(tabs.currentIndex())), name)

    def testPeacockApp(self):
        self.create_app([])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.exe.tabName())
        self.app.main_widget.setTab(self.input.tabName())
        tab = tabs.currentWidget()
        self.assertEqual(tab.MeshPlugin.isEnabled(), False)
        self.assertEqual(tab.vtkwin.isVisible(), False)

    def testPeacockAppWithExe(self):
        self.create_app([Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.exe.tabName())

    def testPeacockAppWithInput(self):
        self.create_app(["../../common/transient.i", Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())
        tab = tabs.currentWidget()
        self.assertEqual(tab.MeshPlugin.isEnabled(), True)
        self.assertEqual(tab.vtkwin.isVisible(), True)

    def check_result(self):
        fname = "peacock_results.png"
        Testing.remove_file(fname)
        Testing.set_window_size(self.vtkwin)
        self.vtkwin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname, allowed=0.92))

    def testResults(self):
        self.create_app(["-r", "gold/out_transient.e"])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.result.tabName())
        self.check_result()

    def testResultsNoOption(self):
        self.create_app(["gold/out_transient.e"])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.result.tabName())
        self.check_result()

    def check_postprocessor(self):
        fname = "peacock_postprocessor.png"
        Testing.remove_file(fname)
        self.assertEqual(self.postprocessor.count(), 1)
        w = self.postprocessor.currentWidget()
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups), 1)
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups[0]._toggles), 3)

        Testing.set_window_size(w.FigurePlugin)
        w.FigurePlugin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

    def check_vector_postprocessor(self):
        fname = "peacock_vector_postprocessor.png"
        Testing.remove_file(fname)
        self.assertEqual(self.vector_postprocessor.count(), 1)
        w = self.vector_postprocessor.currentWidget()
        Testing.process_events(self.qapp, t=1)
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups), 1)
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups[0]._toggles), 7)
        Testing.process_events(self.qapp)

        Testing.set_window_size(w.FigurePlugin)
        w.FigurePlugin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

    def testPostprocessor(self):
        self.create_app(["-p", "../gold/out_transient.csv"])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.postprocessor.tabName())
        self.check_postprocessor()

    def testPostprocessorNoOption(self):
        self.create_app(["../gold/out_transient.csv"])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.postprocessor.tabName())
        self.check_postprocessor()

    def testAllCommandLine(self):
        d = os.getcwd()
        args = ["-i" , "%s/../../common/transient.i" % d,
                "-e", Testing.find_moose_test_exe(),
                "-r", "%s/gold/out_transient.e" % d,
                "-p", "%s/../gold/out_transient.csv" % d,
                "-v", "%s/../gold/time_data_line_sample_*.csv" % d,
                ]
        self.create_app(args)
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.vector_postprocessor.tabName())
        self.check_vector_postprocessor()
        self.app.main_widget.setTab(self.result.tabName())
        Testing.process_events(self.qapp, t=1)
        self.check_result()
        self.app.main_widget.setTab(self.postprocessor.tabName())
        Testing.process_events(self.qapp, t=2)
        self.check_postprocessor()

    def testOnlyInputFileWithExeInPath(self):
        input_file = os.path.abspath('../../common/transient.i')
        dirname = os.path.dirname(Testing.find_moose_test_exe())
        with Testing.remember_cwd():
            os.chdir(dirname)
            args = ["-i", input_file ]
            self.create_app(args)
            tabs = self.app.main_widget.tab_plugin
            self.check_current_tab(tabs, self.input.tabName())
            tab = tabs.currentWidget()
            self.assertEqual(tab.vtkwin.isVisible(), True)
            self.assertEqual(tab.MeshPlugin.isEnabled(), True)

    def testWrongExe(self):
        # use the test/moose_test-opt to try to process a modules/combined input file
        input_file = os.path.join("../../common/transient_heat_test.i")
        self.create_app([input_file, Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())
        tab = tabs.currentWidget()
        self.assertEqual(tab.MeshPlugin.isEnabled(), False)
        self.assertEqual(tab.vtkwin.isVisible(), False)

    def testBadInput(self):
        self.create_app(["-i", "../../common/out_transient.e", Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.exe.tabName())

    def testClearSettings(self):
        args = ["--clear-settings"]
        self.create_app(args)

    def testAutoRun(self):
        self.create_app(["--run", "../../common/transient.i", Testing.find_moose_test_exe(), "-w", os.getcwd()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.result.tabName())
        self.check_result()

if __name__ == '__main__':
    Testing.run_tests()
