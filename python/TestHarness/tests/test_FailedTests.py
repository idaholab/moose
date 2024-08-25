#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
import tempfile
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testFailedTests(self):
        """
        In order to test for failed tests, we need to run run_tests twice. Once
        to create a json file containing previous results, and again to only run
        the test which that has failed.
        """
        with tempfile.TemporaryDirectory() as output_dir:
            args = ['--no-color', '--results-file', 'failed-unittest', '-o', output_dir]
            kwargs = {'tmp_output': False}
            with self.assertRaises(subprocess.CalledProcessError) as cm:
                self.runTests(*args, '-i', 'always_bad', **kwargs)

            e = cm.exception

            self.assertRegex(e.output, r'tests/test_harness.always_ok.*?OK')
            self.assertRegex(e.output, r'tests/test_harness.always_bad.*?FAILED \(CODE 1\)')

            with self.assertRaises(subprocess.CalledProcessError) as cm:
                self.runTests(*args, '--failed-tests', **kwargs)

            e = cm.exception

            # Verify the passing test is not present
            self.assertNotRegex(e.output, r'tests/test_harness.always_ok.*?OK')

            # Verify the caveat represents a previous result
            self.assertRegex(e.output, r'tests/test_harness.always_bad.*?\[PREVIOUS RESULTS: CODE 1\] FAILED \(CODE 1\)')
