import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
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
