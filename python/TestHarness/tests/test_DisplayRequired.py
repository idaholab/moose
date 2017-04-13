import os
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDislpayRequired(self):
        """
        Test that the display required is working.
        """

        display = os.getenv('DISPLAY', None)
        if display:
            os.unsetenv('DISPLAY')

        output = self.runTests('-i', 'display_required')
        self.assertRegexpMatches(output, r'test_harness\.display_required.*?skipped\s\(NO DISPLAY\)')

        if display:
            os.putenv('DISPLAY', display)
