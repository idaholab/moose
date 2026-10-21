# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import subprocess
import sys
import unittest

from TestHarnessTestCase import MOOSE_PYTHON, TestHarnessTestCase

TESTER_DIR = os.path.join(MOOSE_PYTHON, "TestHarness", "testers")
if TESTER_DIR not in sys.path:
    sys.path.append(TESTER_DIR)

from SchemaDiff import SchemaDiff


class TestHarnessTester(TestHarnessTestCase):
    def testSchemaDiff(self):
        output = self.runTests("-i", "schemadiff", exit_code=129).output
        self.assertRegex(
            output, r"test_harness\.schema_jsondiff.*?FAILED \(SCHEMADIFF\)"
        )
        self.assertRegex(
            output, r"test_harness\.schema_xmldiff.*?FAILED \(SCHEMADIFF\)"
        )
        self.assertRegex(
            output, r"test_harness\.schema_invalid_json.*?FAILED \(LOAD FAILED\)"
        )
        self.assertRegex(
            output, r"test_harness\.schema_invalid_xml.*?FAILED \(LOAD FAILED\)"
        )


class TestSchemaDiffStringLists(unittest.TestCase):
    """Test diffing of values stored as whitespace-separated strings."""

    def setUp(self):
        params = SchemaDiff.validParams()
        params["input"] = "input.i"
        # Private param normally set by Parser via Tester.augmentParams()
        params.addPrivateParam("_validation_classes", [])
        self.tester = SchemaDiff("test", params)

    def diff(self, gold, test):
        params = self.tester.specs
        return self.tester.do_deepdiff(
            {"value": gold}, {"value": test}, params["rel_err"], params["abs_zero"]
        )

    def testEqual(self):
        self.assertFalse(self.diff("-1 -2 3", "-1 -2 3"))
        self.assertFalse(self.diff("-1 -2 3", "-1 -2.000001 3"))
        self.assertFalse(self.diff("1e-11 -2", "-1e-11 -2"))

    def testNegativeValues(self):
        self.assertTrue(self.diff("-1 -2", "-1 -3"))
        self.assertTrue(self.diff("1 -2", "1 -2.5"))

    def testAbsZeroDoesNotSkipRemainingValues(self):
        self.assertTrue(self.diff("1e-11 1", "2e-11 2"))
