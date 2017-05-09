import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testTimeout(self):
        """
        Test that timeout tests report TIMEOUT
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'timeout')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.timeout.*?TIMEOUT')
