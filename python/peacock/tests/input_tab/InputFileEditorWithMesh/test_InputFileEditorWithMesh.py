#!/usr/bin/env python
from peacock.Input.InputFileEditorWithMesh import InputFileEditorWithMesh
from PyQt5.QtWidgets import QMainWindow, QMessageBox
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
        self.basic_mesh = "meshrender_basic.png"
        #Testing.remove_file(self.highlight_all)
        #Testing.remove_file(self.highlight_right)
        #Testing.remove_file(self.highlight_left)
        #Testing.remove_file(self.basic_mesh)
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
        w.initialize()
        w.setEnabled(True)
        main_win.show()
        self.assertEqual(w.vtkwin.isVisible(), False)
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
    def testBasic(self):
        main_win, w = self.newWidget()
        self.assertEqual(w.vtkwin.isVisible(), False)
        w.setInputFile(self.input_file)
        self.assertEqual(w.vtkwin.isVisible(), True)
        Testing.set_window_size(w.vtkwin)
        w.vtkwin.onWrite(self.basic_mesh)
        self.assertFalse(Testing.gold_diff(self.basic_mesh))

    def testHighlight(self):
        main_win, w = self.newWidget()
        w.setInputFile(self.input_file)
        self.assertEqual(w.vtkwin.isVisible(), True)
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

    def testHighlightDiffusion(self):
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
        opts = {"cmd_line_options": parser.parse_args([])}
        self.assertEqual(w.vtkwin.isVisible(), False)

        abs_input_file = os.path.abspath(self.input_file)
        opts = {"cmd_line_options": parser.parse_args(['-i', self.input_file])}
        w.initialize(**opts)
        self.assertEqual(w.vtkwin.isVisible(), True)
        self.assertEqual(w.InputFileEditorPlugin.tree.input_filename, abs_input_file)

        w.InputFileEditorPlugin._clearInputFile()
        self.assertEqual(w.vtkwin.isVisible(), False)
        self.assertEqual(w.InputFileEditorPlugin.tree.input_filename, None)

        opts = {"cmd_line_options": parser.parse_args([self.input_file])}
        w.initialize(**opts)
        self.assertEqual(w.vtkwin.isVisible(), True)
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
        w.InputFileEditorPlugin.block_tree.copyBlock(b)
        self.assertEqual(w.InputFileEditorPlugin.has_changed, True)
        self.assertEqual(w.canClose(), False)

        mock_q.return_value = QMessageBox.Yes # The user wants to ignore changes
        self.assertEqual(w.canClose(), True)

        s = tree.getInputFileString()
        self.assertEqual("", s) # AuxVariables isn't included
        b.included = True
        s = tree.getInputFileString()
        self.assertEqual("[AuxVariables]\n  [./New_0]\n  [../]\n[]\n\n", s)

if __name__ == '__main__':
    Testing.run_tests()
