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

    def testDeleted(self):
        """
        Test that deleted tests returns a failed deleted test when extra info argument is supplied
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'deleted', '-e')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.deleted.*?deleted \(test deleted test\)')


    def testDryRun(self):
        """
        Test that --dry-run returns a passing status
        """
        output = self.runTests('-i', 'diffs', '--dry-run')

        self.assertRegexpMatches(output, 'test_harness\.exodiff.*?DRY RUN')
        self.assertRegexpMatches(output, 'test_harness\.csvdiff.*?DRY RUN')

        # Skipped caveat test which returns skipped instead of 'DRY RUN'
        output = self.runTests('-i', 'depend_skip_tests', '--dry-run')
        self.assertIn('skipped (always skipped)', output)

        # NOTE: This test normally returns (skipped dependency). However
        # with dry run, the TestHarness has no idea that this is the case
        # because in order to figure that out, the test has to 'run' and we are
        # not running any tests (its a dry run after all)!
        self.assertRegexpMatches(output, 'test_harness\.needs_a.*?DRY RUN')

        # Deleted caveat test which returns a deleted failing tests while
        # performing a dry run
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'deleted', '-e', '--dry-run')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.deleted.*?deleted \(test deleted test\)')
