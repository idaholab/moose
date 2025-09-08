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
from unittest.mock import patch
import MooseDocs
from MooseDocs.common import project_find


class TestProjectFind(unittest.TestCase):

    @patch(
        "MooseDocs.PROJECT_FILES",
        ["file0.md", "/path/to/another/file0.md", "image1.png"],
    )
    def test(self):
        """
        Test that class with h and C files are located.
        """
        self.assertEqual(project_find("image1.png"), ["image1.png"])
        self.assertEqual(
            project_find("file0.md"), ["file0.md", "/path/to/another/file0.md"]
        )
        self.assertEqual(
            project_find("e0.md"), ["file0.md", "/path/to/another/file0.md"]
        )
        self.assertEqual(project_find("wrong"), [])


if __name__ == "__main__":
    unittest.main(verbosity=2)
