from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testSyntax(self):
        """
        Test for Parser Errors
        """

        # check that parser errors print correctly
        # TODO: Are there more we can test?
        output = self.runExceptionTests('-i', 'parse_errors')
        self.assertIn('duplicate parameter', output)
        self.assertIn('invalid date value', output)
