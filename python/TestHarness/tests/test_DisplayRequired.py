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

        output = self.runTests('--no-color', '-i', 'display_required')
        self.assertRegexpMatches(output, r'test_harness\.display_required.*? \[NO DISPLAY\] SKIP')

        if display:
            os.putenv('DISPLAY', display)
