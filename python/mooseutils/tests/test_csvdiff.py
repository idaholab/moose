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
import unittest
import csv
import tempfile
import argparse
import contextlib
import io
import mooseutils.csvdiff


class TestCsvDiff(unittest.TestCase):
    """
    Test csvdiff
    """

    def setUp(self):
        self._tmpfile0 = tempfile.mkstemp(text=True)[-1]
        with open(self._tmpfile0, "w", encoding="utf-8") as fid:
            csvwriter = csv.writer(fid)
            csvwriter.writerow(["name1", "name2", "name3"])
            csvwriter.writerow(["1", "2.22", "3.3"])
        self._tmpdir = tempfile.TemporaryDirectory()

    def tearDown(self):
        os.remove(self._tmpfile0)
        self._tmpdir.cleanup()

    def _write(self, name, text):
        """Write text to a file in the temporary directory and return its path"""
        path = os.path.join(self._tmpdir.name, name)
        with open(path, "w", encoding="utf-8") as fid:
            fid.write(text)
        return path

    def _writeColumns(self, name, columns):
        """Write a single-row CSV file from a {column: value} dictionary"""
        return self._write(
            name,
            ",".join(columns.keys())
            + "\n"
            + ",".join(map(str, columns.values()))
            + "\n",
        )

    def _numErrors(self, *cli_args):
        """Run CSVDiffer with the given command line arguments and return the number of errors"""
        args = mooseutils.csvdiff.parseArgs(list(cli_args))
        with mooseutils.csvdiff.CSVDiffer(args) as csv_differ:
            csv_differ.diff()
            return csv_differ.getNumErrors()

    def _invalidArgs(self, *cli_args):
        """Return the message printed when parseArgs rejects the command line arguments"""
        out = io.StringIO()
        with contextlib.redirect_stdout(out), self.assertRaises(SystemExit):
            mooseutils.csvdiff.parseArgs(list(cli_args))
        return out.getvalue()

    def _goldAndTest(self):
        """
        Return paths to two files with columns x and y.

        The relative differences are 9.9e-3 for both columns. The absolute difference is 1e-8
        for x and 1e-9 for y.
        """
        gold = self._writeColumns("gold.csv", {"x": 1e-6, "y": 1e-7})
        test = self._writeColumns("test.csv", {"x": 1.01e-6, "y": 1.01e-7})
        return gold, test

    def testSummary(self):
        args = mooseutils.csvdiff.parseArgs(["--summary", self._tmpfile0])
        with mooseutils.csvdiff.CSVSummary(args) as csv_summary:
            self.assertIn(
                "TIME STEPS absolute 0.0 relative 1 floor 0  # min: 0 @ t0  max: 0 @ t0\n\n"
                "GLOBAL VARIABLES absolute 0.0 relative 5.5e-06 floor 1e-10\n"
                "    name1                    # min: 1.000e+00 @ t0          max: 1.000e+00 @ t0\n"
                "    name2                    # min: 2.220e+00 @ t0          max: 2.220e+00 @ t0\n"
                "    name3                    # min: 3.300e+00 @ t0          max: 3.300e+00 @ t0",
                csv_summary.summary(),
            )

    def testSummaryAbsoluteTolerance(self):
        args = mooseutils.csvdiff.parseArgs(
            ["--summary", self._tmpfile0, "--absolute-tolerance", "1e-3"]
        )
        with mooseutils.csvdiff.CSVSummary(args) as csv_summary:
            self.assertIn(
                "GLOBAL VARIABLES absolute 0.001 relative 5.5e-06 floor 1e-10\n",
                csv_summary.summary()[0],
            )

    def testAbsoluteTolerance(self):
        gold, test = self._goldAndTest()
        # Both tolerances exceeded in both columns (one error per column): the default absolute
        # tolerance of zero never passes a difference, and 5e-10 is below both absolute differences
        self.assertEqual(self._numErrors(gold, test), 2)
        self.assertEqual(
            self._numErrors(gold, test, "--absolute-tolerance", "5e-10"), 2
        )
        # The absolute tolerance passes y (1e-9) but not x (1e-8)
        self.assertEqual(self._numErrors(gold, test, "--absolute-tolerance", "5e-9"), 1)
        # Passes through the absolute tolerance alone
        self.assertEqual(self._numErrors(gold, test, "--absolute-tolerance", "1e-7"), 0)
        # Passes through the relative tolerance alone
        self.assertEqual(self._numErrors(gold, test, "--relative-tolerance", "1e-1"), 0)

    def testAbsoluteToleranceMessage(self):
        gold, test = self._goldAndTest()
        args = mooseutils.csvdiff.parseArgs([gold, test])
        with mooseutils.csvdiff.CSVDiffer(args) as csv_differ:
            messages = csv_differ.diff()
        self.assertIn(
            "relative diff = 9.901e-03; absolute diff = 1.000e-08", messages[0]
        )

    def testCustomAbsoluteTolerance(self):
        gold, test = self._goldAndTest()
        # Only y is covered by the override; x uses the global absolute tolerance of zero
        self.assertEqual(
            self._numErrors(
                gold,
                test,
                "--custom-columns",
                "y",
                "--custom-abs-err",
                "1e-8",
            ),
            1,
        )
        # Both columns covered
        self.assertEqual(
            self._numErrors(
                gold,
                test,
                "--custom-columns",
                "x",
                "y",
                "--custom-abs-err",
                "1e-7",
                "1e-8",
            ),
            0,
        )
        # The override is per column: it is too small for x
        self.assertEqual(
            self._numErrors(
                gold,
                test,
                "--absolute-tolerance",
                "1e-7",
                "--custom-columns",
                "x",
                "--custom-abs-err",
                "1e-9",
            ),
            1,
        )

    def testCustomSingleTolerance(self):
        gold, test = self._goldAndTest()
        # A single --custom-* option other than --custom-abs-err is enough
        self.assertEqual(
            self._numErrors(
                gold,
                test,
                "--custom-columns",
                "x",
                "y",
                "--custom-rel-err",
                "1e-1",
                "1e-1",
            ),
            0,
        )
        # A value below the override absolute zero is treated as zero
        self.assertEqual(
            self._numErrors(
                gold,
                test,
                "--custom-columns",
                "x",
                "y",
                "--custom-abs-zero",
                "1e-5",
                "1e-5",
            ),
            0,
        )

    def testComparisonFileAbsoluteTolerance(self):
        gold, test = self._goldAndTest()
        # Global tolerance covers both columns
        cmp_global = self._write(
            "global.cmp",
            "GLOBAL VARIABLES absolute 1e-7 relative 0 floor 0\n    x\n    y\n",
        )
        self.assertEqual(self._numErrors(gold, test, "-c", cmp_global), 0)
        # Alias of the absolute tolerance keyword
        cmp_alias = self._write(
            "alias.cmp",
            "GLOBAL VARIABLES abs_tol 1e-7 relative 0 floor 0\n    x\n    y\n",
        )
        self.assertEqual(self._numErrors(gold, test, "-c", cmp_alias), 0)
        # Field tolerance overrides the global tolerance, which is too small for x
        cmp_field = self._write(
            "field.cmp",
            "GLOBAL VARIABLES absolute 1e-9 relative 0 floor 0\n    x\n    y absolute 1e-8\n",
        )
        self.assertEqual(self._numErrors(gold, test, "-c", cmp_field), 1)
        # No absolute tolerance in the file means exact comparison of both columns
        cmp_none = self._write(
            "none.cmp", "GLOBAL VARIABLES relative 0 floor 0\n    x\n    y\n"
        )
        self.assertEqual(self._numErrors(gold, test, "-c", cmp_none), 2)

    def testSummaryRoundTrip(self):
        gold, test = self._goldAndTest()
        args = mooseutils.csvdiff.parseArgs(
            [
                "--summary",
                gold,
                "--absolute-tolerance",
                "1e-7",
                "--relative-tolerance",
                "0",
            ]
        )
        with mooseutils.csvdiff.CSVSummary(args) as csv_summary:
            summary = csv_summary.summary()[0]
        cmp_file = self._write("summary.cmp", summary)
        self.assertEqual(self._numErrors(gold, test, "-c", cmp_file), 0)

    def testInvalidCustomArgs(self):
        gold, test = self._goldAndTest()
        # --custom-columns without any tolerance
        self.assertIn(
            "at least one of",
            self._invalidArgs(gold, test, "--custom-columns", "x"),
        )
        # Tolerance list shorter than --custom-columns
        self.assertIn(
            "must have the same number of space-separated items",
            self._invalidArgs(
                gold,
                test,
                "--custom-columns",
                "x",
                "y",
                "--custom-abs-err",
                "1e-7",
            ),
        )
        # Any --custom-* option conflicts with a comparison file
        cmp_file = self._write(
            "any.cmp", "GLOBAL VARIABLES relative 0 floor 0\n    x\n"
        )
        self.assertIn(
            "you can not use any --custom-* args",
            self._invalidArgs(
                gold,
                test,
                "-c",
                cmp_file,
                "--custom-columns",
                "x",
                "--custom-abs-err",
                "1e-7",
            ),
        )


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
