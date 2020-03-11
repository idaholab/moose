#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.InputFileEditor import InputFileEditor
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.test_input_file = "../../common/fsp_test.i"
        self.block_list_requested = 0

    def newWidget(self):
        app_info = ExecutableInfo()
        app_info.setPath(Testing.find_moose_test_exe())
        self.assertTrue(app_info.valid())
        w = InputFileEditor()
        w.executableInfoChanged(app_info)
        w.setInputFile(self.test_input_file)
        return w

    def testBlockEditor(self):
        e = self.newWidget()
        mesh = e.tree.getBlockInfo("/Mesh")
        self.assertNotEqual(mesh, None)
        self.assertEqual(e.block_editor, None)
        e._blockEditorRequested(mesh)
        self.assertNotEqual(e.block_editor, None)
        e.block_editor.close()
        self.assertEqual(e.block_editor, None)

    def testBlockList(self):
        e = self.newWidget()
        # this should request variables
        u = e.tree.getBlockInfo("/Preconditioning/FSP/u")
        self.assertNotEqual(u, None)
        e.onNeedBlockList(["/Variables"])
        e._blockEditorRequested(u)
        e.onNeedBlockList(["/Variables"])
        e.closing()
        self.assertEqual(e.block_editor, None)

    def testWrite(self):
        e = self.newWidget()
        u = e.tree.getBlockInfo("/Preconditioning/FSP/u")
        self.assertNotEqual(u, None)
        e._blockEditorRequested(u)
        e.block_editor.close()

        b = e.tree.getBlockInfo("/BCs/left_u")
        self.assertNotEqual(b, None)
        e._blockEditorRequested(b)
        e.block_editor.close()
        e.writeInputFile("fsp_test.i")
        self.compareFiles("fsp_test.i", "gold/fsp_test.i")

if __name__ == '__main__':
    Testing.run_tests()
