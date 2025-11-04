#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

BASE_ARGS = ['--no-color', '-i', 'working_directory']

class TestHarnessTester(TestHarnessTestCase):
    def testWorkingDirectoryGood(self):
        """
        Verify TestHarness can operate in specified subdirectories
        """
        # Test a working scenario
        out = self.runTests(*BASE_ARGS, '--re', 'relative_and_available').output
        self.assertRegex(out, 'tests/test_harness.relative_and_available.*? OK')

    def testDependency(self):
        # Test prereqs are working (check for an OK for the dependency test)
        out = self.runTests(*BASE_ARGS, exit_code=133).output
        self.assertRegex(out, r'tests/test_harness.depend_on_available.*? OK')

    def testAbsolutePath(self):
        # Test we catch an absolute path
        out = self.runTests(*BASE_ARGS, '--re', 'absolute_path', exit_code=128).output
        self.assertRegex(out, r'tests/test_harness.absolute_path.*? FAILED \(ABSOLUTE PATH DETECTED\)')

    def testDirectoryNotFound(self):
        # Test we catch a directory not found
        out = self.runTests(*BASE_ARGS, '--re', 'non_existent', exit_code=132).output
        self.assertRegex(out, r'tests/test_harness.non_existent.*? FAILED \(WORKING DIRECTORY NOT FOUND\)')

    def testExodiff(self):
        ## Specific Testers ##
        # exodiff can access sub directories
        out = self.runTests(*BASE_ARGS, '--re', 'exodiff', exit_code=129).output
        self.assertRegex(out, r'tests/test_harness.exodiff.*? FAILED \(EXODIFF\)')

    def testCSVDiff(self):
        # csvdiff can access sub directories
        out = self.runTests(*BASE_ARGS, '--re', 'csvdiff', exit_code=128).output
        self.assertRegex(out, r'tests/test_harness.csvdiff.*? FAILED \(Override inputs not the same length\)')

    def ValidationTestRunException(self):
        # RunException can access sub directories
        out = self.runTests(*BASE_ARGS, '--re', 'runexception', exit_code=128).output
        self.assertRegex(out, r'tests/test_harness.runexception.*? FAILED \(EXPECTED ERROR MISSING\)')
