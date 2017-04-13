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
        self.assertIn('skipped (skipped dependency)', output)

        # Deleted caveat test which returns a deleted failing tests while
        # performing a dry run
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'deleted', '-e', '--dry-run')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.deleted.*?deleted \(test deleted test\)')
