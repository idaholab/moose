from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testRequiredObjects(self):
        """
        Test that the required_objects check works
        """
        output = self.runTests('-i', 'required_objects')
        self.assertRegexpMatches(output, r'test_harness\.bad_object.*?skipped \(DoesNotExist not found in executable\)')
        self.assertRegexpMatches(output, r'test_harness\.good_objects.*?OK')
        self.checkStatus(output, passed=1, skipped=1)
