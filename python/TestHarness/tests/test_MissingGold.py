import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testMissingGold(self):
        """
        Test for Missing Gold file (Exodus Only)
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'missing_gold')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(MISSING GOLD FILE\)')
