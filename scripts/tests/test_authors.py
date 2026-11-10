#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import mooseutils


class Test(unittest.TestCase):

    def testLocationFolders(self):
        locations = [
            os.path.join(mooseutils.git_root_dir(), "python", "mooseutils"),
            os.path.join(mooseutils.git_root_dir(), "python", "moosesqa"),
        ]

        out = mooseutils.check_output(
            ["./authors.py", *locations, "-j", "1"],
            cwd=os.path.join(mooseutils.git_root_dir(), "scripts"),
        )
        self.assertIn("Andrew", out)
        self.assertIn("C++", out)
        self.assertIn("Python", out)
        self.assertIn("Input", out)
        self.assertIn("Markdown", out)
        self.assertIn("Make", out)
        self.assertIn("YAML", out)
        self.assertIn("Total", out)
        self.assertIn("TOTAL", out)

    def testLanguage(self):
        locations = [os.path.join(mooseutils.git_root_dir(), "python", "mooseutils")]
        out = mooseutils.check_output(
            ["./authors.py", *locations, "-j", "1", "-l", "Python"],
            cwd=os.path.join(mooseutils.git_root_dir(), "scripts"),
        )
        self.assertIn("Andrew", out)
        self.assertNotIn("C++", out)
        self.assertIn("Python", out)
        self.assertNotIn("Input", out)
        self.assertNotIn("Markdown", out)
        self.assertNotIn("Make", out)
        self.assertNotIn("YAML", out)
        self.assertIn("Total", out)
        self.assertIn("TOTAL", out)

    def testSince(self):
        locations = [os.path.join(mooseutils.git_root_dir(), "python", "mooseutils")]

        # Helper that returns the total commit count reported on the TOTAL row.
        # A since-date run only shows the Commits and Merges columns, so the
        # commit total is the second-to-last whitespace-delimited token.
        def total_commits(out):
            total_line = [l for l in out.splitlines() if l.split()[:1] == ["TOTAL"]][0]
            return int(total_line.split()[-2].replace(",", ""))

        def run_since(date):
            return mooseutils.check_output(
                ["./authors.py", *locations, "-j", "1", "--since", date],
                cwd=os.path.join(mooseutils.git_root_dir(), "scripts"),
            )

        # The commit set since a later date is a subset of that since an earlier
        # date, so the earlier date must report strictly more commits. Both dates
        # are in the past, so this history (and these counts) cannot change.
        out_recent = run_since("2024-01-01")
        out_older = run_since("2020-01-01")
        self.assertGreater(total_commits(out_older), total_commits(out_recent))
        # --since drops the all-time line-count columns; only the date-accurate
        # Commits and Merges columns remain in the table header.
        header = [l for l in out_recent.splitlines() if l.split()[:1] == ["Name"]][0]
        self.assertEqual(header.split(), ["Name", "Commits", "Merges"])


if __name__ == "__main__":
    unittest.main(verbosity=2, buffer=True)
