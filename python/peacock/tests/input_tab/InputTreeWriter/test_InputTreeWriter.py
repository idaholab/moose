#!/usr/bin/env python
from peacock.Input.InputTree import InputTree
from peacock.Input import InputTreeWriter
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.basic_input = "../../common/simple_diffusion.i"

    def createBasic(self):
        e = ExecutableInfo()
        e.setPath(Testing.find_moose_test_exe())
        t = InputTree(e)
        t.setInputFile(self.basic_input)
        return t

    def testWriter(self):
        t = self.createBasic()
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion.i")

    def testInactive(self):
        t = self.createBasic()
        d = t.getBlockInfo("/Kernels/diff")
        d.included = False
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion_no_diff.i")
        k = t.getBlockInfo("/Kernels")
        k.included = False
        b = t.getBlockInfo("/BCs")
        b.included = False
        e = t.getBlockInfo("/Executioner")
        e.included = False
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion_inactive.i")

        d.included = True
        k.included = True
        b.included = True
        e.included = True
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion.i")

if __name__ == '__main__':
    Testing.run_tests()
