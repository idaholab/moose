import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testExpect(self):
        """
        Test that Expect Err/Out tests report if the message they are supposed to look for is missing
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'expect')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'test_harness\.no_expect_err.*?FAILED \(EXPECTED ERROR MISSING\)')
        self.assertRegexpMatches(e.output, r'test_harness\.no_expect_out.*?FAILED \(EXPECTED OUTPUT MISSING\)')
        self.assertRegexpMatches(e.output, r'test_harness\.absent_out.*?FAILED \(OUTPUT NOT ABSENT\)')


    def testExpectMissing(self):
        """
        Test that Expect Err/Out tests report an error if both expect_err and expect_assert are missing.
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
             self.runTests('-i', 'expect_missing_params')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'Either "expect_err" or "expect_assert" must be supplied')
