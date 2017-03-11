import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDiffs(self):
        """
        Test for Exodiffs, CSVDiffs
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'diffs')

        e = cm.exception
        self.assertRegexpMatches(e.output, r'test_harness\.exodiff.*?FAILED \(EXODIFF\)')
        self.assertRegexpMatches(e.output, r'test_harness\.csvdiff.*?FAILED \(CSVDIFF\)')
        self.assertRegexpMatches(e.output, r'test_harness\.exodiff.*?Running exodiff')
        self.assertRegexpMatches(e.output, r'test_harness\.csvdiff.*?Running CSVDiffer.py')
