#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testRequiredApps(self):
        """
        Test that the required_apps check works
        """
        result = self.runTests('--no-color', '-i', 'required_apps')

        out = result.output
        self.assertRegex(out, r'test_harness\.bad_app.*? \[APP DOESNOTEXIST NOT REGISTERED IN EXECUTABLE\] SKIP')
        self.assertRegex(out, r'test_harness\.good_app.*? OK')

        self.checkStatus(result.harness, passed=1, skipped=1)
