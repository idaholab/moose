import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testUnknownPrereq(self):
        """
        Test for Unknown Prereq (cyclic error)
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'unknown_prereq')

        e = cm.exception
        self.assertEqual(e.returncode, 1)
        self.assertIn('Cyclic or Invalid Dependency Detected!', e.output)
