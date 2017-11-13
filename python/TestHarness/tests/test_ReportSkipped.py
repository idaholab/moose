from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testSyntax(self):
        """
        Test for --no-report (do not report skipped tests)
        """

        # Verify the skipped test _does_ appear
        output = self.runExceptionTests('-i', 'ignore_skipped')
        self.assertIn('skipped (always skipped)', output)

        # Verify the skipped test does _not_ appear
        output = self.runTests('--no-report', '-i', 'ignore_skipped')
        self.assertNotIn('skipped (always skipped)', output)
