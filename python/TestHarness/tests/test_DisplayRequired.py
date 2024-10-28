#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
        self.assertRegex(output, r'test_harness\.display_required.*? \[NO DISPLAY\] SKIP')

        if display:
            os.putenv('DISPLAY', display)
