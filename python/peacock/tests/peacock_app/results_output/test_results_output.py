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
import os
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.oversample_filename = "check_oversample.png"
        self.pressure_filename = "pressure.png"
        self.globals_filename = "globals.png"
        Testing.remove_file(self.oversample_filename)
        Testing.remove_file(self.pressure_filename)
        Testing.remove_file(self.globals_filename)
        Testing.remove_file("with_date.e")
        Testing.remove_file("over.e")
        Testing.clean_files()

    def checkInputFile(self, input_file, image_name, exe_path=None, cwd=None):
        Testing.remove_file(image_name)
        args = ["-w"]
        if cwd:
            args.append(cwd)
            args.append(os.path.join(cwd, input_file))
        else:
            args.append(self.starting_directory)
            args.append(input_file)

        if exe_path:
            args.append(exe_path)
        else:
            args.append(Testing.find_moose_test_exe())

        app = self.createPeacockApp(args)
        result_plugin = app.main_widget.tab_plugin.ExodusViewer
        exe_plugin = app.main_widget.tab_plugin.ExecuteTabPlugin
        vtkwin = result_plugin.currentWidget().VTKWindowPlugin
        app.main_widget.setTab(result_plugin.tabName())
        Testing.set_window_size(vtkwin)
        Testing.remove_file("peacock_run_exe_tmp_out.e")
        exe_plugin.ExecuteRunnerPlugin.runClicked()
        # make sure we are finished
        while not self.finished:
            self.qapp.processEvents()
        Testing.process_events(t=3)
        Testing.set_window_size(vtkwin)
        vtkwin.onWrite(image_name)
        self.assertFalse(Testing.gold_diff(image_name))

    def testOversample(self):
        self.checkInputFile("../../common/oversample.i", self.oversample_filename)

    def testDate(self):
        self.checkInputFile("../../common/transient_with_date.i", self.oversample_filename)

    def testPressure(self):
        """
        There was a problem processing pressure_test.i input file. "type" was incorrectly getting
        output on /BCs/Pressure causing a failure.
        Make sure it is fixed.
        """
        image_name = os.path.abspath(self.pressure_filename)
        with Testing.remember_cwd():
            pressure_dir = os.path.join(os.environ["MOOSE_DIR"], "modules", "tensor_mechanics", "test", "tests", "pressure")
            exe = Testing.find_moose_test_exe("modules/combined", "combined")
            self.checkInputFile("pressure_test.i", image_name, exe_path=exe, cwd=pressure_dir)

    def testGlobalParams(self):
        """
        Issue #127
        Global params wasn't being read.
        Make sure it is fixed.
        """
        image_name = os.path.abspath(self.globals_filename)
        with Testing.remember_cwd():
            reconstruct_dir = os.path.join(os.environ["MOOSE_DIR"], "modules", "phase_field", "test", "tests", "reconstruction")
            exe = Testing.find_moose_test_exe("modules/combined", "combined")
            self.checkInputFile("2phase_reconstruction2.i", image_name, exe_path=exe, cwd=reconstruct_dir)

if __name__ == '__main__':
    Testing.run_tests()
