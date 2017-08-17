from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testIgnore(self):
        """
        Test that --ignore overrides normally skipped tests
        """
        # Run a skipped test
        output = self.runTests('-i', 'ignore_skipped', '--ignore', 'skip')
        self.assertRegexpMatches(output, 'test_harness\.ignore_skipped.*?OK')

        # Run a skipped heavy test
        output = self.runTests('-i', 'ignore_heavy', '--ignore', 'heavy')
        self.assertRegexpMatches(output, 'test_harness\.ignore_heavy.*?OK')

        # Run a skipped compiler test
        output = self.runTests('-i', 'ignore_compiler', '--ignore', 'compiler')
        self.assertRegexpMatches(output, 'test_harness\.ignore_compiler.*?OK')

        # Run a skipped platform test
        output = self.runTests('-i', 'ignore_platform', '--ignore', 'platform')
        self.assertRegexpMatches(output, 'test_harness\.ignore_platform.*?OK')

        # Run a skipped prereq test
        output = self.runTests('-i', 'ignore_prereq', '--ignore', 'prereq')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?skipped')
        self.assertRegexpMatches(output, 'test_harness\.ignore_skipped_dependency.*?OK')

        # Check that a dependency test runs when its prereq test is skipped
        output = self.runTests('-i', 'ignore_prereq', '--ignore', 'skip')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?OK')
        self.assertRegexpMatches(output, 'test_harness\.ignore_skipped_dependency.*?OK')

        # Run a multiple caveat skipped test by manually supplying each caveat
        output = self.runTests('-i', 'ignore_multiple', '--ignore', 'skip heavy compiler platform')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multiple.*?OK')

        # Run a multiple caveat skipped test using built in default 'all'
        output = self.runTests('-i', 'ignore_multiple', '--ignore')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multiple.*?OK')

        # Skip a multiple caveat test by not supplying enough caveats to ignore
        output = self.runTests('-i', 'ignore_multiple', '--ignore', 'skip heavy compiler')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multiple.*?skipped')

        # Run a multiple caveat prereq test using built in default 'all'
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?OK')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multi_prereq_dependency.*?OK')

        # Run a multiple caveat prereq test by manually supplying each caveat
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore', 'prereq skip heavy compiler platform')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?OK')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multi_prereq_dependency.*?OK')

        # Skip a multiple caveat prereq test by not supplying enough caveats to ignore
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore', 'prereq skip heavy compiler')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?OK')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multi_prereq_dependency.*?skipped')

        # Check that a multiple caveat dependency test runs when its prereq test is skipped
        # This test may seem redundant, but `prereq` is handled differently than the other caveats
        output = self.runTests('-i', 'ignore_multiple_prereq', '--ignore', 'prereq heavy compiler platform')
        self.assertRegexpMatches(output, 'test_harness\.always_skipped.*?skipped')
        self.assertRegexpMatches(output, 'test_harness\.ignore_multi_prereq_dependency.*?OK')
