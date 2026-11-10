#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
import os
import tempfile
import unittest
import mock

from moosesqa import SQAReport, SQAUnitTestReport


class TestSQAUnitTestReport(unittest.TestCase):
    def setUp(self):
        self._temporary_directory = tempfile.TemporaryDirectory()
        self.root = self._temporary_directory.name
        self.source = os.path.join(self.root, "unit", "src", "Example.C")
        os.makedirs(os.path.dirname(self.source), exist_ok=True)
        with open(self.source, "w", encoding="utf-8") as stream:
            stream.write("TEST(ExampleSuite, exampleCase) {}\n")
        self.manifest = os.path.join(self.root, "doc", "legacy_unit_tests.yml")

    def tearDown(self):
        self._temporary_directory.cleanup()

    def _report(self, tracked_files):
        return SQAUnitTestReport(
            title="example",
            unit_root=self.root,
            legacy_manifest=self.manifest,
            tracked_files=tracked_files,
        )

    @mock.patch("mooseutils.colorText", side_effect=lambda t, c, **kwargs: t)
    def testPass(self, color_text):
        os.makedirs(os.path.dirname(self.manifest), exist_ok=True)
        with open(self.manifest, "w", encoding="utf-8") as stream:
            stream.write("unit:\n  - ExampleSuite.exampleCase\n")

        report = self._report([self.source, self.manifest])
        text = report.getReport()
        self.assertEqual(report.status, SQAReport.Status.PASS)
        self.assertIn("OK", text)
        self.assertIn("unit_test_sqa: 0", text)

    @mock.patch("mooseutils.colorText", side_effect=lambda t, c, **kwargs: t)
    def testError(self, color_text):
        report = self._report([self.source])
        text = report.getReport()
        self.assertEqual(report.status, SQAReport.Status.ERROR)
        self.assertIn("FAIL", text)
        self.assertIn("unit_test_sqa: 1", text)
        self.assertIn("has no SQA metadata", text)


if __name__ == "__main__":
    unittest.main(verbosity=2)
