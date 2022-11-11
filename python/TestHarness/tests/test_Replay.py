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
        output_a = self.runTests('--verbose', '--timing', '-i', 'always_ok', '--results-file', 'unittest_Replay')
        output_b = self.runTests('--verbose', '--timing', '--show-last-run', '--results-file', 'unittest_Replay')
        compile = self.reCompile()
        formated_a = compile.findall(str(output_a))
        formated_b = compile.findall(str(output_b))

        if formated_a != formated_b:
            self.fail(f'--show-last-run did not match last run\n\n{formated_a}\n\n{formated_b}')

    def testDiffReplay(self):
        """ Verify that the feature fails when asked to capture new output """
        output_a = self.runTests('--verbose', '--timing', '-i', 'always_ok', '--results-file', 'unittest_Replay')
        # --re=doesenotexist will produce no output (or rather different output than the above)
        output_b = self.runTests('--verbose', '--timing', '--show-last-run', '--results-file', 'unittest_Replay', '--re=doesnotexist')
        compile = self.reCompile()
        formated_a = compile.findall(str(output_a))
        formated_b = compile.findall(str(output_b))

        if formated_a == formated_b:
            self.fail(f'--show-last-run matched when it should not have')

    def testNoResultsFile(self):
        """ Verify the TestHarness errors correctly when there is no results file to work with """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--show-last-run', '--results-file', 'non_existent')
        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'A previous run does not exist')
