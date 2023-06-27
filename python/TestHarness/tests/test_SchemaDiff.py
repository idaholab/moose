#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import io
import mock
import deepdiff
import TestHarness
from TestHarnessTestCase import TestHarnessTestCase
from contextlib import redirect_stdout

# To allow tester inheriting
import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),
                                             os.path.pardir,
                                             'testers' )))
from TestHarness.testers import SchemaDiff

class TestHarnessTester(TestHarnessTestCase):
    def testSchemaDiff(self):
        """ Verify SchemaDiff detects diffs and malformed diff files """
        output = self.runExceptionTests('-i', 'schemadiff')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_csvdiff_absolute.*?FAILED \(CSVDIFF\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_csvdiff_relative.*?FAILED \(CSVDIFF\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_csvdiff_relative_override.*?FAILED \(CSVDIFF\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_jsondiff.*?FAILED \(JSONDIFF\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_xmldiff.*?FAILED \(XMLDIFF\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_invalid_json.*?FAILED \(LOAD FAILED\)')
        self.assertRegex(output.decode('utf-8'), r'test_harness\.schema_invalid_xml.*?FAILED \(LOAD FAILED\)')

    def testSchemaDiffCSVOutput(self):
        """ Verify SchemaDiff / CSVDIFF prints correct error message """
        # Verify we used and failed at Absolute Diff Checking
        output = self.runExceptionTests('-i', 'schemadiff', '--re', 'schema_csvdiff_absolute')
        self.assertRegex(output.decode('utf-8'), r'.*?Absolute Error Tolorance:')
        self.assertNotRegex(output.decode('utf-8'), r'.*?Relative Error Tolorance:')

        # Verify we used and failed at Relative Diff Checking
        output = self.runExceptionTests('-i', 'schemadiff', '--re', 'schema_csvdiff_relative')
        self.assertRegex(output.decode('utf-8'), r'.*?Relative Error Tolorance:')
        self.assertNotRegex(output.decode('utf-8'), r'.*?Absolute Error Tolorance:')

        # Verify we used and failed at Relative Diff Checking while even though specifying a useless
        # abs_err param while not setting rel_err to 0
        output = self.runExceptionTests('-i', 'schemadiff', '--re', 'schema_csvdiff_relative_override')
        self.assertRegex(output.decode('utf-8'), r'.*?Relative Error Tolorance:')
        self.assertNotRegex(output.decode('utf-8'), r'.*?Absolute Error Tolorance:')

    @mock.patch.object(deepdiff, 'DeepDiff')
    @mock.patch.object(SchemaDiff.SchemaDiff, 'do_deepdiff')
    def testException(self, schemadiff_mock, deepdiff_mock):
        """
        Verify TestHarness handles deepdiff exceptions properly

        Note: 'schemadiff_mock' intentionally not used. We are trying to verify what happens
              when deepdiff.DeepDiff raises an exception. But in order to Mock deepdiff using
              mock.patch, one needs to include the object that imports deepdiff. Ref:
              https://docs.python.org/3/library/unittest.mock.html#where-to-patch
        """
        deepdiff_mock.side_effect = Exception('BOOM!')
        MOOSE_DIR = os.getenv('MOOSE_DIR')
        os.chdir(f'{MOOSE_DIR}/test')
        out = io.StringIO()
        with redirect_stdout(out):
            harness = TestHarness.TestHarness(['', '-i', 'schemadiff', '-c'], MOOSE_DIR)
            harness.findAndRunTests()
        self.assertRegex(out.getvalue(), r'test_harness\.schema_xmldiff.*?FAILED \(DeepDiff exception\)')
        self.assertRegex(out.getvalue(), r'test_harness\.schema_jsondiff.*?FAILED \(DeepDiff exception\)')
