import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testCyclic(self):
        """
        Test cyclic dependency error.
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'cyclic_tests')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'tests/test_harness.testA.*?FAILED \(Cyclic or Invalid Dependency Detected!\)')
        self.assertRegexpMatches(e.output, r'tests/test_harness.testB.*?skipped \(skipped dependency\)')
        self.assertRegexpMatches(e.output, r'tests/test_harness.testC.*?skipped \(skipped dependency\)')
