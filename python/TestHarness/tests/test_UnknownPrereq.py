import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testUnknownPrereq(self):
        """
        Test for Unknown Prereq
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'unknown_prereq')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'tests/test_harness.foo.*?FAILED \(unknown dependency\)')
