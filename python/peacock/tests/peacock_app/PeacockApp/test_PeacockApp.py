#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import unittest
from peacock.utils import Testing
import os
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def tearDown(self):
        if self.input:
            self.input.MeshViewerPlugin._reset()
        super(Tests, self).tearDown()

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

    def testPeacockAppWithExe(self):
        self.create_app([Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())

    def testPeacockAppWithInput(self):
        self.create_app(["transient.i", Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())

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
        Testing.process_events(t=1)
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups), 1)
        self.assertEqual(len(w.PostprocessorSelectPlugin._groups[0]._toggles), 7)
        Testing.process_events()

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
        args = ["-i" , "transient.i",
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
        Testing.process_events(t=1)
        self.check_result()
        self.app.main_widget.setTab(self.postprocessor.tabName())
        Testing.process_events(t=2)
        self.check_postprocessor()

    def testOnlyInputFileWithExeInPath(self):
        input_file = os.path.abspath('transient.i')
        dirname = os.path.dirname(Testing.find_moose_test_exe())
        with Testing.remember_cwd():
            os.chdir(dirname)
            args = ["-i", input_file ]
            self.create_app(args)
            tabs = self.app.main_widget.tab_plugin
            self.check_current_tab(tabs, self.input.tabName())

    def testWrongExe(self):
        # use the test/moose_test-opt to try to process a modules/combined input file
        input_file = "transient_heat_test.i"
        self.create_app([input_file, Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())

    def testBadInput(self):
        self.create_app(["-i", "gold/out_transient.e", Testing.find_moose_test_exe()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())

    def testClearSettings(self):
        args = ["--clear-settings"]
        self.create_app(args)

    def testAutoRun(self):
        Testing.remove_file("out_transient.e")
        self.create_app(["--run", "transient.i", Testing.find_moose_test_exe(), "-w", os.getcwd()])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.result.tabName())
        self.check_result()

    def testBadMesh(self):
        self.create_app(["bad_mesh.i", Testing.find_moose_test_exe(), "-w", os.getcwd()])
        tabs = self.app.main_widget.tab_plugin
        t = self.input.InputFileEditorPlugin.tree
        self.check_current_tab(tabs, self.input.tabName())
        self.assertEqual(self.input.vtkwin.isEnabled(), False)

        # make sure highlighting doesn't crash peacock
        bc = t.getBlockInfo("/BCs/left")
        self.input.highlightChanged(bc)

        # Disable the mesh block
        mesh = t.getBlockInfo("/Mesh")
        mesh.included = False
        self.input.blockChanged(mesh)
        self.assertEqual(self.input.vtkwin.isEnabled(), False) # still disabled

        mesh.included = True
        self.input.blockChanged(mesh)
        self.assertEqual(self.input.vtkwin.isEnabled(), False) # still disabled

        p = mesh.getParamInfo("dim")
        p.value = '2'
        self.input.blockChanged(mesh)
        self.assertEqual(self.input.vtkwin.isEnabled(), True) # Mesh should be good now

        # Disabling the mesh block should disable the mesh view
        mesh.included = False
        self.input.blockChanged(mesh)
        #self.assertEqual(self.input.vtkwin.isEnabled(), False)

    def testExodusChangedFile(self):
        """
        When changing input files, both having the default exodus output file name,
        we were seeing that the MeshPlugin wasn't getting reset properly.
        """
        cwd = os.getcwd()
        diffusion1 = "simple_diffusion.i"
        self.create_app([diffusion1, Testing.find_moose_test_exe(), "-w", cwd])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())
        self.app.main_widget.setTab(self.exe.tabName())
        self.exe.ExecuteRunnerPlugin.runClicked()
        Testing.process_events(t=2)
        self.app.main_widget.setTab(self.result.tabName())
        mesh = self.result.currentWidget().MeshPlugin
        Testing.process_events(t=2)

        self.assertTrue(mesh.isEnabled())
        mesh.ScaleX.setValue(.9)
        mesh.ScaleY.setValue(.8)
        mesh.ScaleZ.setValue(.7)
        mesh.Representation.setCurrentIndex(1)
        mesh.DisplacementToggle.setChecked(True)
        mesh.DisplacementMagnitude.setValue(2.0)
        fname = "diffusion1.png"
        Testing.set_window_size(self.vtkwin)
        self.vtkwin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

        diffusion2 = "simple_diffusion2.i"
        self.app.main_widget.setTab(self.input.tabName())
        self.input.setInputFile(diffusion2)
        self.app.main_widget.setTab(self.exe.tabName())
        self.exe.ExecuteOptionsPlugin.setWorkingDir(cwd)
        self.exe.ExecuteRunnerPlugin.runClicked()
        Testing.process_events(t=2)
        self.app.main_widget.setTab(self.result.tabName())

        Testing.process_events(t=2)
        self.assertTrue(mesh.isEnabled())
        self.assertEqual(mesh.ViewMeshToggle.isChecked(), False)
        self.assertEqual(mesh.ScaleX.value(), 1.0)
        self.assertEqual(mesh.ScaleY.value(), 1.0)
        self.assertEqual(mesh.ScaleZ.value(), 1.0)
        self.assertEqual(mesh.Representation.currentIndex(), 0)
        self.assertEqual(mesh.DisplacementToggle.isChecked(), True)
        self.assertEqual(mesh.DisplacementMagnitude.value(), 1.0)

        fname = "diffusion2.png"
        Testing.set_window_size(self.vtkwin)
        self.vtkwin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

    @unittest.skip("#11756")
    def testExodusExtentsReload(self):
        """
        Test that the extents is reloaded after changing input and then re-running.
        """
        fname = "extentsOn.png"
        if os.path.exists(fname):
            os.remove(fname)

        # Create Peacock instance
        cwd = os.getcwd()
        diffusion1 = "simple_diffusion.i"
        self.create_app([diffusion1, Testing.find_moose_test_exe(), "-w", cwd])
        tabs = self.app.main_widget.tab_plugin
        self.check_current_tab(tabs, self.input.tabName())

        # Run simple diffusion
        self.app.main_widget.setTab(self.exe.tabName())
        self.exe.ExecuteRunnerPlugin.runClicked()
        Testing.process_events(t=1)

        # Enable extents and compare output
        self.app.main_widget.setTab(self.result.tabName())
        mesh = self.result.currentWidget().MeshPlugin
        Testing.process_events(t=1)
        mesh.Extents.setChecked(True)
        mesh.Extents.stateChanged.emit(True)
        Testing.process_events(t=1)

        # Write file
        Testing.set_window_size(self.vtkwin)
        self.vtkwin.onWrite(fname)
        self.assertFalse(Testing.gold_diff(fname))

        # Re-execute
        self.app.main_widget.setTab(self.exe.tabName())
        self.exe.ExecuteRunnerPlugin.runClicked()
        Testing.process_events(t=1)

        # Show result and write file again
        self.app.main_widget.setTab(self.result.tabName())
        Testing.process_events(t=1)
        self.vtkwin.onWrite('extentsOn.png')
        self.assertFalse(Testing.gold_diff(fname, allowed=0.99))

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
