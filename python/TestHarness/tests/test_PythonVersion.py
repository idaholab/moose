# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testVersion(self):
        """Test that python=... is working."""
        output = self.runTests('-i', 'python_version').output
        self.assertIn('[PYTHON != 2]', output)
        self.assertIn('[PYTHON != 3.5]', output)
        self.assertIn('[PYTHON != 3.4.1]', output)
