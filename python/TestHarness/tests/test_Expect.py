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
        self.assertRegexpMatches(e.output, r'test_harness\.no_expect_err.*?FAILED \(NO EXPECTED ERR\)')
        self.assertRegexpMatches(e.output, r'test_harness\.no_expect_out.*?FAILED \(EXPECTED OUTPUT MISSING\)')
