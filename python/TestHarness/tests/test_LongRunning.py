from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testLongRunningStatus(self):
        """
        Test for RUNNING status in the TestHarness
        """
        output = self.runTests('-i', 'long_running')
        self.assertIn('RUNNING...', output)
        self.assertIn('[FINISHED]', output)
