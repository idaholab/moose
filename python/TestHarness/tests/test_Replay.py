#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import subprocess
import shutil
import tempfile

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def setUp(self):
        """
        setUp occurs before every test. Clean up previous results file
        """
        self.output_dir = os.path.join(os.getenv('MOOSE_DIR'), 'test', 'unittest_Replay')

        try:
            # remove previous results file
            shutil.rmtree(self.output_dir)
        except:
            pass

    def tearDown(self):
        """
        tearDown occurs after every test.
        """
        self.setUp()

    def reCompile(self):
        # Capture everything up to the final tally.
        # Which includes TestHarness total run time, and will always be different
        return re.compile(r'(.*)Final Test Results', re.MULTILINE)

    def testReplay(self):
        """ Test ability to replay back previous run results """
        with tempfile.TemporaryDirectory() as output_dir:
            base_args = ['--verbose', '-c', '--timing', '--results-file', 'unittest_Replay', '-o', output_dir]
            base_kwargs = {'tmp_output': False}
            output_a = self.runTests(*base_args, '-i', 'always_ok', **base_kwargs)
            output_b = self.runTests(*base_args, '--show-last-run', **base_kwargs)

        # The only difference should be the total run time, so replace the run time
        # from the first with the run time from the second
        def parseSummary(output):
            search = re.search(r'Ran (\d+) tests in (\d+.\d+) seconds', output)
            self.assertTrue(search is not None)
            return int(search.group(1)), float(search.group(2))
        num_tests, total_time = parseSummary(output_a)
        other_num_tests, other_total_time = parseSummary(output_b)
        self.assertEqual(num_tests, other_num_tests)
        output_b = output_b.replace(f'Ran {num_tests} tests in {other_total_time} seconds',
                                    f'Ran {num_tests} tests in {total_time} seconds')
        self.assertEqual(output_a, output_b)

    def testDiffReplay(self):
        """ Verify that the feature fails when asked to capture new output """
        with tempfile.TemporaryDirectory() as output_dir:
            base_args = ['--verbose', '--timing', '--results-file', 'unittest_Replay', '-o', output_dir]
            base_kwargs = {'tmp_output': False}
            output_a = self.runTests(*base_args, '-i', 'always_ok', **base_kwargs)
            # --re=doesenotexist will produce no output (or rather different output than the above)
            output_b = self.runTests(*base_args, '--show-last-run', '--re=doesnotexist', **base_kwargs)
        self.assertIn('Ran 1 tests in', output_a)
        self.assertIn('Ran 0 tests in', output_b)

    def testNoResultsFile(self):
        """ Verify the TestHarness errors correctly when there is no results file to work with """
        with tempfile.TemporaryDirectory() as output_dir:
            with self.assertRaises(subprocess.CalledProcessError) as cm:
                self.runTests('--show-last-run', '--results-file', 'non_existent', '-o', output_dir, tmp_output=False)
            e = cm.exception
            self.assertIn(f'The previous run {output_dir}/non_existent does not exist', e.output)
