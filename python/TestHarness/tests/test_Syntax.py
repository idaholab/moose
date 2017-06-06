from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testSyntax(self):
        """
        Test for SYNTAX PASS status in the TestHarness
        """
        output = self.runTests('-i', 'syntax')
        self.assertIn('SYNTAX PASS', output)
