#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import json, os, subprocess, tempfile
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def tearDown(self):
        """
        tearDown occurs after every test.
        """
        self.setUp()

    def checkFilesExist(self, output_dir, tests, output_object_names, spec_file):
        # The directories within the test directory where these tests reside
        test_folder = os.path.join('tests', 'test_harness')
        # The complete path to the directory where the tests reside
        test_base_path = os.path.join(os.getenv('MOOSE_DIR'), 'test', test_folder)
        # The complete path where the output should reside
        output_base_path = os.path.join(output_dir, test_folder)

        # Load the previous results
        with open(os.path.join(output_dir, '.previous_test_results.json')) as f:
            results = json.load(f)
        test_results = results['tests']
        # The test folder should be in the results
        test_spec_results = test_results[test_folder]
        # The number of tests in the test spec should be the number provided
        self.assertEqual(len(tests), len(test_spec_results['tests']))
        # Test spec should match
        self.assertEqual(os.path.join(test_base_path, spec_file), test_spec_results['spec_file'])

        for test in tests:
            # Test should be in the results
            test_results = test_spec_results['tests'][test]
            # Get the output files from the test spec
            result_output_files = test_results['output_files']
            # Make sure each output file exists and is set in the results file
            for name in output_object_names:
                output_path = f'{output_base_path}/{test}.{name}_out.txt'
                self.assertTrue(os.path.exists(output_path))
                self.assertEqual(result_output_files[name], output_path)
            # And make sure that we don't have output from any other objects
            for name, output_path in result_output_files.items():
                if name not in output_object_names:
                    self.assertEqual(output_path, None)

    def testWriteAll(self):
        """ Test write all output files --sep-files """
        with tempfile.TemporaryDirectory() as output_dir:
            with self.assertRaises(subprocess.CalledProcessError):
                self.runTests('--no-color', '-i', 'diffs', '--sep-files', '-o', output_dir, tmp_output=False)
            self.checkFilesExist(output_dir, ['csvdiff', 'exodiff'], ['runner_run', 'tester'], 'diffs')

        with tempfile.TemporaryDirectory() as output_dir:
            self.runTests('--no-color', '-i', 'always_ok', '--sep-files', '-o', output_dir, tmp_output=False)
            self.checkFilesExist(output_dir, ['always_ok'], ['runner_run'], 'always_ok')
