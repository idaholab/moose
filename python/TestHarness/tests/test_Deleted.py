import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDeleted(self):
        """
        Test that deleted tests returns a failed deleted test when extra info argument is supplied
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'deleted', '-e')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.deleted.*?deleted \(test deleted test\)')
