#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from unittest import mock

import MooseDocs


class TestInitLargeMedia(unittest.TestCase):
    def setUp(self):
        self._orig_project_files = MooseDocs.PROJECT_FILES
        MooseDocs.PROJECT_FILES = set()

    def tearDown(self):
        MooseDocs.PROJECT_FILES = self._orig_project_files

    def testEnabled(self):
        """init_large_media(True) initializes the submodule and indexes its files."""
        with (
            mock.patch("mooseutils.git_init_submodule") as git_init_submodule,
            mock.patch.object(MooseDocs, "ls_files") as ls_files,
        ):
            ls_files.return_value = {"large_media/foo.png"}
            MooseDocs.init_large_media(True)

        git_init_submodule.assert_called_once_with(
            "large_media", MooseDocs.MOOSE_DIR, True
        )
        ls_files.assert_called_once()
        self.assertIn("large_media/foo.png", MooseDocs.PROJECT_FILES)

    def testDisabled(self):
        """init_large_media(False) skips submodule initialization and file indexing."""
        with (
            mock.patch("mooseutils.git_init_submodule") as git_init_submodule,
            mock.patch.object(MooseDocs, "ls_files") as ls_files,
        ):
            MooseDocs.init_large_media(False)

        git_init_submodule.assert_not_called()
        ls_files.assert_not_called()
        self.assertEqual(MooseDocs.PROJECT_FILES, set())


if __name__ == "__main__":
    unittest.main(verbosity=2)
