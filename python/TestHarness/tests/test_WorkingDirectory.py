#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testWorkingDirectoryGood(self):
        """
        Verify TestHarness can operate in specified subdirectories
        """
        # Test a working scenario
        output = self.runTests('--no-color', '-i', 'working_directory', '--re', 'relative_and_available')
        self.assertRegex(output.decode('utf-8'), 'tests/test_harness.relative_and_available.*? OK')


    def testDependency(self):
        # Test prereqs are working (check for an OK for the dependency test)
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.depend_on_available.*? OK')

    def testAbsolutePath(self):
        # Test we catch an absolute path
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory', '--re', 'absolute_path')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.absolute_path.*? FAILED \(ABSOLUTE PATH DETECTED\)')

    def testDirectoryNotFound(self):
        # Test we catch a directory not found
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory', '--re', 'non_existent')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.non_existent.*? FAILED \(WORKING DIRECTORY NOT FOUND\)')

    def testExodiff(self):
        ## Specific Testers ##
        # exodiff can access sub directories
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory', '--re', 'exodiff')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.exodiff.*? FAILED \(EXODIFF\)')

    def testCSVDiff(self):
        # csvdiff can access sub directories
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory', '--re', 'csvdiff')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.csvdiff.*? FAILED \(Override inputs not the same length\)')

    def testRunException(self):
        # RunException can access sub directories
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'working_directory', '--re', 'runexception')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.runexception.*? FAILED \(EXPECTED ERROR MISSING\)')
