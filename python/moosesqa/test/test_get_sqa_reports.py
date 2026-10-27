#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

import os
import shutil
import subprocess
import tempfile
import unittest

from moosesqa import get_sqa_unit_test_reports


@unittest.skipUnless(shutil.which("git"), "git is not available")
class TestGetSQAUnitTestReportsRootTitle(unittest.TestCase):
    def setUp(self):
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = self._temporary_directory.name
        subprocess.run(["git", "init", "-q"], cwd=self.root, check=True)
        unit_src = os.path.join(self.root, "unit", "src")
        os.makedirs(unit_src)
        with open(os.path.join(unit_src, "Example.C"), "w") as fid:
            fid.write("TEST(Example, test) {}\n")
        subprocess.run(["git", "add", "-A"], cwd=self.root, check=True)

    def tearDown(self):
        self._temporary_directory.cleanup()

    def testDownstreamApp(self):
        # No 'framework/' directory: the root-level report is named for the app's own
        # repository, not hardcoded to 'framework'.
        reports = get_sqa_unit_test_reports(root_dir=self.root)
        self.assertEqual(len(reports), 1)
        self.assertEqual(reports[0].title, os.path.basename(self.root))

    def testMooseRepository(self):
        os.makedirs(os.path.join(self.root, "framework"))
        reports = get_sqa_unit_test_reports(root_dir=self.root)
        self.assertEqual(len(reports), 1)
        self.assertEqual(reports[0].title, "framework")


if __name__ == "__main__":
    unittest.main(verbosity=2)
