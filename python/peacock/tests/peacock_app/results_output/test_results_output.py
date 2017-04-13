#!/usr/bin/env python
from peacock.utils import Testing
import os

class Tests(Testing.PeacockTester):
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
        args = [input_file]
        if exe_path:
            args.append(exe_path)
        else:
            args.append(Testing.find_moose_test_exe())
        app = self.createPeacockApp(args)
        if cwd:
            os.chdir(cwd)
        result_plugin = app.main_widget.tab_plugin.ExodusViewer
        exe_plugin = app.main_widget.tab_plugin.ExecuteTabPlugin
        vtkwin = result_plugin.currentWidget().VTKWindowPlugin
        app.main_widget.setTab(result_plugin.tabName())
        Testing.set_window_size(vtkwin)
        exe_plugin.ExecuteRunnerPlugin.runClicked()
        # make sure we are finished
        while not self.finished:
            self.qapp.processEvents()
        Testing.process_events(self.qapp, t=2)
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
            pressure_dir = os.path.join(os.environ["MOOSE_DIR"], "modules", "tensor_mechanics", "tests", "pressure")
            exe = Testing.find_moose_test_exe("modules/combined", "combined")
            os.chdir(pressure_dir)
            Testing.remove_file("peacock_run_exe_tmp_out.e")
            self.checkInputFile("pressure_test.i", image_name, exe_path=exe, cwd=os.getcwd())

    def testGlobalParams(self):
        """
        Issue #127
        Global params wasn't being read.
        Make sure it is fixed.
        """
        image_name = os.path.abspath(self.globals_filename)
        with Testing.remember_cwd():
            reconstruct_dir = os.path.join(os.environ["MOOSE_DIR"], "modules", "phase_field", "tests", "reconstruction")
            exe = Testing.find_moose_test_exe("modules/combined", "combined")
            os.chdir(reconstruct_dir)
            Testing.remove_file("peacock_run_exe_tmp_out.e")
            self.checkInputFile("2phase_reconstruction2.i", image_name, exe_path=exe, cwd=os.getcwd())

if __name__ == '__main__':
    Testing.run_tests()
