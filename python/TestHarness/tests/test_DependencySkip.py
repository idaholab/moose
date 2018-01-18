from TestHarnessTestCase import TestHarnessTestCase
class TestHarnessTester(TestHarnessTestCase):
    def testDependencySkip(self):
        """
        Test skipping a test if its prereq is also skipped
        """
        output = self.runTests('--no-color', '-i', 'depend_skip_tests')
        self.assertIn('[ALWAYS SKIPPED] SKIP', output)
        self.assertIn('[SKIPPED DEPENDENCY] SKIP', output)
