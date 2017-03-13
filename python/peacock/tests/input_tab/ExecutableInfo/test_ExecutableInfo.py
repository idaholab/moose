#!/usr/bin/env python
import unittest
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing

class Tests(Testing.PeacockTester):
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
        self.assertIn("ODETimeDerivative", out)

    def testPickle(self):
        exe_path = Testing.find_moose_test_exe()
        e = ExecutableInfo()
        e.clearCache()
        e.setPath(exe_path)
        p = e.toPickle()
        e2 = ExecutableInfo()
        e2.fromPickle(p)
        self.assertEqual(e2.path_map, e.path_map)

    def testDump(self):
        e = ExecutableInfo()
        e.setPath(Testing.find_moose_test_exe(dirname="modules/combined", exe_base="combined"))
        p = e.path_map.get("/Preconditioning")
        self.assertNotEqual(p, None)
        self.assertEqual(p.star, True)
        self.assertEqual(p.hard, True)

if __name__ == '__main__':
    unittest.main()
