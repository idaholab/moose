#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testRequiredObjects(self):
        """
        Test that the required_objects check works
        """
        output = self.runTests('--no-color', '-i', 'required_objects')
        self.assertRegex(output, r'test_harness\.bad_object.*? \[DOESNOTEXIST NOT FOUND IN EXECUTABLE\] SKIP')
        self.assertRegex(output, r'test_harness\.good_objects.*? OK')
        self.checkStatus(output, passed=1, skipped=1)
