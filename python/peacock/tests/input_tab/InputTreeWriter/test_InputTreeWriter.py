#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
        self.checkFile(s, "gold/simple_diffusion.i", True)

    def testInactive(self):
        t = self.createBasic()
        d = t.getBlockInfo("/Kernels/diff")
        d.included = False
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion_no_diff.i", True)
        k = t.getBlockInfo("/Kernels")
        k.included = False
        b = t.getBlockInfo("/BCs")
        b.included = False
        e = t.getBlockInfo("/Executioner")
        e.included = False
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion_inactive.i", True)

        d.included = True
        k.included = True
        b.included = True
        e.included = True
        s = InputTreeWriter.inputTreeToString(t.root)
        self.checkFile(s, "gold/simple_diffusion.i")

if __name__ == '__main__':
    Testing.run_tests()
