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
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def checkFile(self, output, gold_file, write_output=False):
        if write_output:
            with open("tmp_out.txt", "w") as f:
                f.write(output)

        with open(gold_file, "r") as f:
            gold_output = f.read()
            self.assertEqual(gold_output, output)

    def testInfo(self):
        e = ExecutableInfo()
        e.clearCache()
        e.setPath("")
        self.assertFalse(e.valid())
        e.setPath("no_exist")
        self.assertFalse(e.valid())

        exe_path = Testing.find_moose_test_exe()
        e.setPath(exe_path)
        self.assertTrue(e.valid())

        e.setPath(exe_path)
        self.assertTrue(e.valid())

        e.setPath("")
        self.assertTrue(e.valid())

        e.setPath("no_exist")
        self.assertFalse(e.valid())

        # this should hit the cache
        e.setPath(exe_path)
        self.assertTrue(e.valid())

    def testTree(self):
        e = ExecutableInfo()
        e.clearCache()
        exe_path = Testing.find_moose_test_exe()
        e.setPath(exe_path)
        root = e.path_map["/"]
        self.assertIn("Mesh", root.children_list)
        m = root.children["Mesh"]
        self.assertEqual(m.hard, True)
        self.assertEqual(e.path_map["/Mesh"], m)

        out = e.dumpDefaultTree(hard_only=False)
        self.assertIn("Partitioner", out)
        self.assertIn("Partitioner", out)
        self.assertIn("ScalarKernels", out)
        self.assertNotIn("DirichletBC", out)

    def testPickle(self):
        exe_path = Testing.find_moose_test_exe()
        e = ExecutableInfo()
        e.clearCache()
        e.setPath(exe_path)
        p = e.toPickle()
        e2 = ExecutableInfo()
        e2.fromPickle(p)
        self.assertEqual(e2.path_map, e.path_map)

    def checkPath(self, e, path, star, hard):
        p = e.path_map.get(path)
        self.assertNotEqual(p, None)
        self.assertEqual(p.star, star)
        self.assertEqual(p.hard, hard)

    def testCombined(self):
        e = ExecutableInfo()
        e.setPath(Testing.find_moose_test_exe(dirname="modules/combined", exe_base="combined"))
        self.checkPath(e, "/Preconditioning", True, True)
        self.checkPath(e, "/BCs", True, True)
        self.checkPath(e, "/BCs/Pressure", True, True)
        self.checkPath(e, "/Adaptivity", False, True)
        self.checkPath(e, "/Adaptivity/Markers", True, True)
        self.checkPath(e, "/GlobalParams", False, True)
        self.checkPath(e, "/Mesh", True, True)
        self.checkPath(e, "/AuxVariables", True, True)
        self.checkPath(e, "/AuxVariables/*/InitialCondition", False, False)
        self.checkPath(e, "/Variables/*/InitialCondition", False, False)

if __name__ == '__main__':
    unittest.main()
