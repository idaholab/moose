import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testShouldExecute(self):
        """
        Test should_execute logic
        """

        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'should_execute')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'test_harness\.should_execute_true_ok.*?OK')
        self.assertRegexpMatches(e.output, r'test_harness\.should_execute_false_ok.*?OK')
        self.assertRegexpMatches(e.output, r'test_harness\.should_execute_true_fail.*?FAILED \(EXODIFF\)')
