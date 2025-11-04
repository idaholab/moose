#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testLongestJobs(self):
        """
        Test for --longest-jobs in the TestHarness with 2 passing jobs, 1 failing job, and 1 skipped job.
        """
        output = self.runTests('-i', 'longest_jobs', '--longest-jobs', '4', exit_code=128).output

        self.assertIn('4 Longest Running Jobs', output)
        self.assertRegex(output, r'(?s)Longest Running Jobs.*run_1')
        self.assertRegex(output, r'(?s)Longest Running Jobs.*run_2')
        self.assertRegex(output, r'(?s)Longest Running Jobs.*run_fail')
        self.assertNotRegex(output, r'(?s)Longest Running Jobs.*run_skip')

    def testLongestJobsNoneCompleted(self):
        """
        Test for --longest-jobs in the TestHarness with no jobs ran.
        """
        output = self.runTests('-i', 'longest_jobs', '--re', 'foo', '--longest-jobs', '100').output
        self.assertNotIn('Longest Running Jobs', output)
