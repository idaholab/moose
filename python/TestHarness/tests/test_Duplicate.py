import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDuplicateOutputs(self):
        """
        Test for duplicate output files in the same directory
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'duplicate_outputs')

        e = cm.exception

        # skip case
        self.assertRegexpMatches(e.output, 'FATAL TEST HARNESS ERROR')

    def testDuplicateOutputsOK(self):
        """
        Test for duplicate output files in the same directory
        """
        output = self.runTests('-i', 'duplicate_outputs_ok')
        output += self.runTests('-i', 'duplicate_outputs_ok', '--heavy')

        # skip case
        self.assertNotRegexpMatches(output, 'skipped_out.e')
        # heavy case
        self.assertNotRegexpMatches(output, 'heavy_out.e')
        # all
        self.assertNotRegexpMatches(output, 'FATAL TEST HARNESS ERROR')
