#!/usr/bin/env python
from PyQt5.QtCore import Qt
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
    def setUp(self):
        super(Tests, self).setUp()
        Testing.clean_files()

    def testRunResult(self):
        args = ["../../common/transient.i", Testing.find_moose_test_exe()]
        app = self.createPeacockApp(args)
        exe_plugin = app.main_widget.tab_plugin.ExecuteTabPlugin
        pp_plugin = app.main_widget.tab_plugin.PostprocessorViewer
        exe_plugin.ExecuteOptionsPlugin.csv_checkbox.setCheckState(Qt.Checked)
        app.main_widget.setTab(exe_plugin.tabName())
        exe_plugin.ExecuteRunnerPlugin.runClicked()

        # make sure we are finished
        while not self.finished:
            self.qapp.processEvents()
        app.main_widget.setTab(pp_plugin.tabName())
        pp = pp_plugin.currentWidget()

        Testing.process_events(self.qapp, t=3)
        self.assertEqual(len(pp.PostprocessorSelectPlugin._groups), 1)
        self.assertEqual(len(pp.PostprocessorSelectPlugin._groups[0]._toggles), 3)

if __name__ == '__main__':
    Testing.run_tests()
