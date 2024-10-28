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
    def testLongestJobs(self):
        """
        Test for --longest-jobs in the TestHarness with 2 passing jobs, 1 failing job, and 1 skipped job.
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'longest_jobs', '--longest-jobs', '4')

        output = cm.exception.output

        self.assertIn('4 longest running jobs', output)
        self.assertRegex(output, r'(?s)longest running jobs.*run_1')
        self.assertRegex(output, r'(?s)longest running jobs.*run_2')
        self.assertRegex(output, r'(?s)longest running jobs.*run_fail')
        self.assertNotRegex(output, r'(?s)longest running jobs.*run_skip')

    def testLongestJobsNoneCompleted(self):
        """
        Test for --longest-jobs in the TestHarness with no jobs ran.
        """
        output = self.runTests('-i', 'longest_jobs', '--re', 'foo', '--longest-jobs', '100')

        self.assertIn('100 longest running jobs', output)
        self.assertNotRegex(output, r'(?s)longest running jobs.*<No jobs were completed>')
