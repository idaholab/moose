#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.InputFileEditorWithMesh import InputFileEditorWithMesh
from PyQt5.QtWidgets import QMainWindow, QMessageBox, QApplication
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing
import argparse, os
from mock import patch

class BaseTests(Testing.PeacockTester):
    def setUp(self):
        super(BaseTests, self).setUp()
        self.input_file = "../../common/transient.i"
        self.highlight_left = "meshrender_highlight_left.png"
        self.highlight_right = "meshrender_highlight_right.png"
        self.highlight_all = "meshrender_highlight_all.png"
        self.highlight_block = "meshrender_highlight_block.png"
        self.highlight_nodes = "meshrender_highlight_nodes.png"
        self.highlight_dup = "meshrender_highlight_dup.png"
        self.basic_mesh = "meshrender_basic.png"
        self.mesh_toggle = "meshrender_toggle.png"
        self.mesh_toggle_disable = "meshrender_toggle_disable.png"

        Testing.remove_file(self.highlight_all)
        Testing.remove_file(self.highlight_right)
        Testing.remove_file(self.highlight_left)
        Testing.remove_file(self.highlight_block)
        Testing.remove_file(self.highlight_nodes)
        Testing.remove_file(self.highlight_dup)
        Testing.remove_file(self.basic_mesh)
        Testing.remove_file(self.mesh_toggle)
        Testing.remove_file(self.mesh_toggle_disable)
        Testing.clean_files()
        self.num_time_steps = None
        self.time_step_changed_count = 0

    def newWidget(self):
        main_win = QMainWindow()
        w = InputFileEditorWithMesh(size=[640,640])
        main_win.setCentralWidget(w)
        app_info = ExecutableInfo()
        app_info.setPath(Testing.find_moose_test_exe())
        self.assertTrue(app_info.valid())
        w.onExecutableInfoChanged(app_info)
        menubar = main_win.menuBar()
        menubar.setNativeMenuBar(False)
        w.addToMainMenu(menubar)
        w.setEnabled(True)
        main_win.show()
        w.numTimeStepsChanged.connect(self.timeStepChanged)
        return main_win, w

    def timeStepChanged(self, num_steps):
        self.num_time_steps = num_steps
        self.time_step_changed_count += 1

    def compareGold(self, w, filename):
        w.input_filename = filename
        Testing.remove_file(filename)
        w.InputFileEditorPlugin.writeInputFile(filename)
        self.compareFiles(filename)

    def compareFiles(self, test_file):
        gold_file = "gold/%s" % test_file
        test_data = ""
        gold_data = ""
        with open(test_file, "r") as f:
            test_data = f.read()

        with open(gold_file, "r") as f:
            gold_data = f.read()
        self.assertEqual(test_data, gold_data)

class Tests(BaseTests):
    qapp = QApplication([])

    def testBasic(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.basic_mesh)
        self.assertFalse(Testing.gold_diff(self.basic_mesh))

    def testBCHighlight(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        tree = w.InputFileEditorPlugin.tree
        b = tree.getBlockInfo("/BCs/left")
        self.assertNotEqual(b, None)
        self.assertEqual(b.getParamInfo("boundary").value, '3')
        w.blockChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.highlight_left)
        self.assertFalse(Testing.gold_diff(self.highlight_left))

        b = tree.getBlockInfo("/BCs/right")
        self.assertNotEqual(b, None)
        self.assertEqual(b.getParamInfo("boundary").value, '1')
        w.blockChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.highlight_right)
        self.assertFalse(Testing.gold_diff(self.highlight_right))

        b = tree.getBlockInfo("/BCs/all")
        self.assertNotEqual(b, None)
        self.assertEqual(b.getParamInfo("boundary").value, '0 1 2 3')
        w.blockChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.highlight_all)
        self.assertFalse(Testing.gold_diff(self.highlight_all))

        b= tree.getBlockInfo("/Executioner")
        self.assertNotEqual(b, None)
        w.highlightChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.basic_mesh)
        self.assertFalse(Testing.gold_diff(self.basic_mesh))

    def testBCHighlightDiffusion(self):
        self.input_file = "../../common/simple_diffusion.i"
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        self.assertEqual(w.vtkwin.isVisible(), True)
        tree = w.InputFileEditorPlugin.tree
        b = tree.getBlockInfo("/BCs/left")
        self.assertNotEqual(b, None)
        self.assertEqual(b.getParamInfo("boundary").value, 'left')
        w.highlightChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.highlight_left)
        self.assertFalse(Testing.gold_diff(self.highlight_left))

        b = tree.getBlockInfo("/BCs/right")
        self.assertNotEqual(b, None)
        self.assertEqual(b.getParamInfo("boundary").value, 'right')
        w.highlightChanged(b)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.highlight_right)
        self.assertFalse(Testing.gold_diff(self.highlight_right))

    def testFromOptions(self):
        parser = argparse.ArgumentParser()
        parser.add_argument('arguments',
                type=str,
                metavar="N",
                nargs="*",
                )
        InputFileEditorWithMesh.commandLineArgs(parser)
        main_win, w = self.newWidget()

        abs_input_file = os.path.abspath(self.input_file)
        w.initialize(parser.parse_args(['-i', self.input_file]))
        self.assertEqual(w.InputFileEditorPlugin.tree.input_filename, abs_input_file)

        w.InputFileEditorPlugin._clearInputFile()
        self.assertEqual(w.InputFileEditorPlugin.tree.input_filename, None)

        w.initialize(parser.parse_args([self.input_file]))
        self.assertEqual(w.InputFileEditorPlugin.tree.input_filename, abs_input_file)

    def testBlockChanged(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        tree = w.InputFileEditorPlugin.tree
        b = tree.getBlockInfo("/Mesh")
        self.assertNotEqual(b, None)
        w.blockChanged(b)

        b = tree.getBlockInfo("/Executioner")
        self.assertNotEqual(b, None)
        self.assertEqual(self.time_step_changed_count, 1)
        self.assertEqual(self.num_time_steps, 8)
        w.blockChanged(b)
        self.assertEqual(self.time_step_changed_count, 2)
        self.assertEqual(self.num_time_steps, 8)

        num_steps_param = b.getParamInfo("num_steps")
        self.assertNotEqual(num_steps_param, None)
        # was 5, add 5 more
        num_steps_param.value = 10
        w.blockChanged(b)
        self.assertEqual(self.time_step_changed_count, 3)
        self.assertEqual(self.num_time_steps, 13)

        b.included = False
        w.blockChanged(b)
        self.assertEqual(self.time_step_changed_count, 4)
        self.assertEqual(self.num_time_steps, 0)

    @patch.object(QMessageBox, "question")
    def testCanClose(self, mock_q):
        mock_q.return_value = QMessageBox.No
        main_win, w = self.newWidget()
        self.assertEqual(w.canClose(), True)
        tree = w.InputFileEditorPlugin.tree

        w.setInputFile(self.input_file)
        self.assertEqual(w.canClose(), True)

        b = tree.getBlockInfo("/Mesh")
        self.assertNotEqual(b, None)
        w.InputFileEditorPlugin.blockChanged.emit(b, w.InputFileEditorPlugin.tree)
        self.assertEqual(w.InputFileEditorPlugin.has_changed, True)
        self.assertEqual(w.canClose(), False)

        w.setInputFile(self.input_file)
        self.assertEqual(w.canClose(), True)

    @patch.object(QMessageBox, "question")
    def testAddBlock(self, mock_q):
        """
        Tests to make sure adding a block to the InputFileEditor
        actually updates the input file.
        """
        mock_q.return_value = QMessageBox.No # The user doesn't want to ignore changes
        main_win, w = self.newWidget()
        tree = w.InputFileEditorPlugin.tree

        self.assertEqual(w.canClose(), True)

        b = tree.getBlockInfo("/AuxVariables")
        self.assertNotEqual(b, None)
        self.assertEqual(b.included, False)
        self.assertFalse(b.wantsToSave())
        w.InputFileEditorPlugin.block_tree.copyBlock(b)
        self.assertEqual(b.included, True)
        self.assertTrue(b.wantsToSave())
        self.assertEqual(w.InputFileEditorPlugin.has_changed, True)
        self.assertEqual(w.canClose(), False)

        mock_q.return_value = QMessageBox.Yes # The user wants to ignore changes
        self.assertEqual(w.canClose(), True)

        new_block = "[AuxVariables]\n  [New_0]\n  []\n[]\n"
        s = tree.getInputFileString()
        self.assertEqual(new_block, s)

    def testBlockHighlight(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        bh = w.BlockHighlighterPlugin
        Testing.set_window_size(w.vtkwin)

        bh.BlockSelector.Options.setCurrentText("0")
        w.vtkwin.onWrite(self.highlight_block)
        self.assertFalse(Testing.gold_diff(self.highlight_block))

        bh.BlockSelector.Options.setCurrentText("")

        bh.SidesetSelector.Options.setCurrentText("right")
        w.vtkwin.onWrite(self.highlight_right)
        self.assertFalse(Testing.gold_diff(self.highlight_right))

        bh.SidesetSelector.Options.setCurrentText("left")
        w.vtkwin.onWrite(self.highlight_left)
        self.assertFalse(Testing.gold_diff(self.highlight_left))

        bh.SidesetSelector.Options.setCurrentText("")

        bh.NodesetSelector.Options.setCurrentText("left")
        w.vtkwin.onWrite(self.highlight_nodes)
        self.assertFalse(Testing.gold_diff(self.highlight_nodes))

    def testBlockHighlightDup(self):
        """
        Make sure we don't regress https://github.com/idaholab/moose/issues/11404
        """
        main_win, w = self.newWidget()
        tree = w.InputFileEditorPlugin.tree
        Testing.set_window_size(w.vtkwin)
        mesh = tree.getBlockInfo("/Mesh")
        mesh.setBlockType("GeneratedMesh")
        mesh.setParamValue("dim", "3")
        mesh.included = True
        w.blockChanged(mesh)
        Testing.process_events(t=1)
        w.blockChanged(mesh) # We need to do it again to trigger the original bug
        bh = w.BlockHighlighterPlugin
        bh.SidesetSelector.Options.setCurrentText("bottom")

        w.MeshViewerPlugin.onCameraChanged((-0.7786, 0.2277, 0.5847), (-2, -2, -1), (0, 0, 0))
        w.vtkwin.onWrite(self.highlight_dup)
        self.assertFalse(Testing.gold_diff(self.highlight_dup))

    def testMeshToggle(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        tree = w.InputFileEditorPlugin.tree
        b = tree.getBlockInfo("/Mesh")
        self.assertNotEqual(b, None)
        self.assertTrue(b.included)
        w.blockChanged(b)

        w.vtkwin.onWrite(self.mesh_toggle)
        self.assertFalse(Testing.gold_diff(self.mesh_toggle))

        b.included = False
        self.assertFalse(b.included)
        w.blockChanged(b)
        w.vtkwin.onWrite(self.mesh_toggle_disable)
        self.assertFalse(Testing.gold_diff(self.mesh_toggle_disable))

        b.included = True
        self.assertTrue(b.included)
        w.blockChanged(b)
        w.vtkwin.onWrite(self.mesh_toggle)
        self.assertFalse(Testing.gold_diff(self.mesh_toggle))

    def testMeshCameraWithChangedInputFiles(self):
        """
        Previously we always wrote out the temporary mesh file to the same
        filename. This caused problems with changing input files when the camera
        had changed on one of them. The camera wouldn't reset so the new mesh
        could potentially be in a weird position.
        """
        main_win, w = self.newWidget()
        sdiffusion = "simple_diffusion.i"
        w.setInputFile(sdiffusion)
        w.MeshViewerPlugin.onCameraChanged((-0.7786, 0.2277, 0.5847), (-2, -2, -1), (0, 0, 0))
        sdiffusion_image = "meshrender_camera_moved.png"
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(sdiffusion_image)
        self.assertFalse(Testing.gold_diff(sdiffusion_image, .98))

        other = "spherical_average.i"
        w.setInputFile(other)
        other_image = "meshrender_3d.png"
        w.MeshViewerPlugin.onCameraChanged((0.2, 0.5, 0.8), (-30, 10, -20), (0, 5, 0))
        w.vtkwin.onWrite(other_image)
        self.assertFalse(Testing.gold_diff(other_image, .93))

        w.setInputFile(sdiffusion)
        w.vtkwin.onWrite(sdiffusion_image)
        self.assertFalse(Testing.gold_diff(sdiffusion_image, .93))

        w.setInputFile(other)
        w.vtkwin.onWrite(other_image)
        self.assertFalse(Testing.gold_diff(other_image, .93))

if __name__ == '__main__':
    Testing.run_tests()
