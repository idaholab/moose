import os
import sys
import unittest
from unittest import mock
import tempfile
import shutil

from TestHarness.TestHarness import readTestRoot, TestHarness
from TestHarness.tests.TestHarnessTestCase import MOOSE_DIR

class TestAppendPythonPath(unittest.TestCase):
    def setUp(self):
        self.dir = os.path.realpath(tempfile.mkdtemp())
        self.orig_cwd = os.getcwd()

    def tearDown(self):
        shutil.rmtree(self.dir, ignore_errors=True)
        os.chdir(self.orig_cwd)

    def test_readTestRoot(self):
        # Create paths to set as extra pythonpaths
        new_paths = ["fake_package", "mock_package"]
        for path in new_paths:
            os.mkdir(os.path.join(self.dir, path))

        # Create testroot file
        content = [
            "app_name = test_harness_test\n",
            "extra_pythonpath = {}\n".format(":".join(new_paths)),
        ]
        testroot_path = os.path.join(self.dir, "testroot")
        with open(testroot_path, "w") as fid:
            fid.writelines(content)

        # Run method
        app_name, args, hit_node, extra_pythonpath = readTestRoot(testroot_path)
        self.assertEqual(app_name, "test_harness_test")
        self.assertEqual(args, [])
        self.assertIsNotNone(hit_node)
        self.assertEqual(len(extra_pythonpath), len(new_paths))
        for res, exp in zip(extra_pythonpath, new_paths):
            self.assertEqual(res, os.path.join(self.dir, exp))

        # Show that missing package with raise an error
        shutil.rmtree(os.path.join(self.dir, new_paths[0]))
        with self.assertRaisesRegex(FileNotFoundError, f"Could not find .*/{new_paths[0]}"):
            readTestRoot(testroot_path)

    def test_append_pythonpath_from_testroot(self):
        # Create paths to set as extra pythonpaths
        new_paths = ["fake_package", "mock_package"]
        new_paths_abs = [os.path.join(self.dir, path) for path in new_paths]
        for path in new_paths_abs:
            os.mkdir(path)

        # Create testroot file
        content = [
            "app_name = test_harness_test\n",
            "extra_pythonpath = {}\n".format(":".join(new_paths)),
        ]
        testroot_path = os.path.join(self.dir, "testroot")
        with open(testroot_path, "w") as fid:
            fid.writelines(content)

        # Create a fake executioner
        app_name = "test_harness_test"
        with open(os.path.join(self.dir, f"{app_name}-opt"), "w") as fid:
            fid.write("")

        # Go into directory and build TestHarness
        os.chdir(self.dir)
        TestHarness.build(
            ["unused", "--no-capabilities"], "test_harness_test", MOOSE_DIR
        )
        for path in new_paths_abs:
            self.assertEqual(sys.path.count(path), 1)

    def test_append_pythonpath_from_executable(self):
        # Add a fake package in the directory to grab
        fake_package = os.path.join(self.dir, "python")
        os.mkdir(fake_package)

        # Create a fake executioner
        app_name = "test_harness_test"
        with open(os.path.join(self.dir, f"{app_name}-opt"), "w") as fid:
            fid.write("")

        # Build the TestHarness, which should add to path
        os.chdir(self.dir)
        TestHarness.build(["unused", "--no-capabilities"], app_name, MOOSE_DIR, skip_testroot=True)
        self.assertEqual(sys.path.count(os.path.join(self.dir, "python")), 1)


if __name__ == '__main__':
    unittest.main()
