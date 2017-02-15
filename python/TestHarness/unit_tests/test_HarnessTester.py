import os
import unittest
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

    def testDependencySkip(self):
        """
        Test skipping a test if its prereq is also skipped
        """
        output = self.runTests('-i', 'depend_skip_tests')

        self.assertIn('skipped (always skipped)', output)
        self.assertIn('skipped (skipped dependency)', output)

    def testLongRunningStatus(self):
        """
        Test for RUNNING status in the TestHarness
        """
        output = self.runTests('-i', 'long_running')

        self.assertIn('RUNNING...', output)
        self.assertIn('[FINISHED]', output)

    def testDiffs(self):
        """
        Test for Exodiffs, CSVDiffs
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'diffs')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(EXODIFF\)')
        self.assertRegexpMatches(e.output, 'test_harness\.csvdiff.*?FAILED \(CSVDIFF\)')
        self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?Running exodiff')
        self.assertRegexpMatches(e.output, 'test_harness\.csvdiff.*?Running CSVDiffer.py')

    def testMissingGold(self):
        """
        Test for Missing Gold file (Exodus Only)
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'missing_gold')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(MISSING GOLD FILE\)')

    def testExpect(self):
        """
        Test that Expect Err/Out tests report if the message they are supposed to look for is missing
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'expect')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.no_expect_err.*?FAILED \(NO EXPECTED ERR\)')
        self.assertRegexpMatches(e.output, 'test_harness\.no_expect_out.*?FAILED \(EXPECTED OUTPUT MISSING\)')
