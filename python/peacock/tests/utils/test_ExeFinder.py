#!/usr/bin/env python
from peacock.utils import ExeFinder
from peacock.utils import Testing
import os

class Tests(Testing.PeacockTester):
    def test_searchForExe(self):
        exe = ExeFinder.searchForExe()
        self.assertEqual(exe, None)

        method = os.environ.get("METHOD", "opt")
        moose_dir = os.environ.get("MOOSE_DIR")
        self.assertNotEqual(moose_dir, None)
        start_dir = os.path.join(moose_dir, "test")
        exe = ExeFinder.searchForExe(start_dir=start_dir)
        self.assertNotEqual(exe, None)
        self.assertTrue(exe.endswith("-%s" % method))

        exe = ExeFinder.searchForExe(start_dir=start_dir, methods=["unknown"])
        self.assertEqual(exe, None)

        for method in ["opt", "dbg", "oprof", "devel"]:
            exe_path = os.path.join(start_dir, "moose_test-%s" % method)
            if not os.path.exists(exe_path):
                continue
            os.environ["METHOD"] = method
            exe = ExeFinder.searchForExe(start_dir=start_dir)
            self.assertEqual(exe, exe_path)
            self.assertNotEqual(exe, None)
            self.assertTrue(exe.endswith("-%s" % method))

        # unset METHOD, it should pick up the opt version
        os.environ.pop("METHOD")
        exe = ExeFinder.searchForExe(start_dir=start_dir)
        self.assertNotEqual(exe, None)
        self.assertTrue(exe.endswith("-opt"))

    def test_recursiveFindFile(self):
        start_dir = os.path.dirname(os.path.abspath(__file__))
        exe = ExeFinder.recursiveFindFile(start_dir, "run_tests")
        self.assertNotEqual(exe, None)
        self.assertEqual(os.path.basename(exe), "run_tests")

        exe = ExeFinder.recursiveFindFile(start_dir, "really_does_not_exist")
        self.assertEqual(exe, None)

        exe = ExeFinder.recursiveFindFile(start_dir, "really_does_not_exist", problems_dir="peacock")
        self.assertEqual(exe, None)

if __name__ == '__main__':
    Testing.run_tests()
