from TestHarnessTestCase import TestHarnessTestCase
class TestHarnessTester(TestHarnessTestCase):
    def testDependencySkip(self):
        """
        Test skipping a test if its prereq is also skipped
        """
        output = self.runTests('-i', 'depend_skip_tests')
        self.assertIn('skipped (always skipped)', output)
        self.assertIn('skipped (skipped dependency)', output)
