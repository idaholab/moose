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
from PyQt5.QtCore import Qt
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.transient_png = "check_transient.png"
        Testing.remove_file(self.transient_png)
        self.diffusion_png = "check_diffusion.png"
        Testing.remove_file(self.diffusion_png)
        Testing.clean_files()

    def run_and_check(self, app, filename):
        exe_plugin = app.main_widget.tab_plugin.ExecuteTabPlugin
        exe_plugin.ExecuteOptionsPlugin.csv_checkbox.setCheckState(Qt.Checked)
        result_plugin = app.main_widget.tab_plugin.ExodusViewer
        app.main_widget.setTab(exe_plugin.tabName())
        exe_plugin.ExecuteOptionsPlugin.setWorkingDir(self.starting_directory)
        exe_plugin.ExecuteRunnerPlugin.runClicked()
        app.main_widget.setTab(result_plugin.tabName())

        vtkwin = result_plugin.currentWidget().VTKWindowPlugin
        Testing.set_window_size(vtkwin)
        # make sure we are finished
        while not self.finished:
            self.qapp.processEvents()
        Testing.process_events(t=5)
        app.main_widget.setTab(result_plugin.tabName())
        Testing.set_window_size(vtkwin)
        Testing.process_events(t=1)
        vtkwin.onWrite(filename)
        self.assertFalse(Testing.gold_diff(filename))
        return app

    def checkTransient(self):
        args = ["../../common/transient.i", Testing.find_moose_test_exe()]
        app = self.createPeacockApp(args)
        self.run_and_check(app, self.transient_png)
        return app

    def testRunResult(self):
        self.checkTransient()

    def testChangeResultFilename(self):
        app = self.checkTransient()
        app.main_widget.tab_plugin.InputFileEditorWithMesh.setInputFile("../../common/simple_diffusion.i")
        self.run_and_check(app, self.diffusion_png)

if __name__ == '__main__':
    Testing.run_tests()
