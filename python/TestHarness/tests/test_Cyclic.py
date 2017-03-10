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
        self.assertEqual(e.returncode, 1)
        self.assertIn('Cyclic or Invalid Dependency Detected!', e.output)
        self.assertIn('tests/test_harness.testA', e.output)
        self.assertIn('tests/test_harness.testB', e.output)
        self.assertIn('tests/test_harness.testC', e.output)
