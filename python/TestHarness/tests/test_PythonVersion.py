# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
from TestHarness import checkPythonVersion


class TestHarnessTester(TestHarnessTestCase):
    def testMinimumVersion(self):
        """Test that the TestHarness rejects unsupported Python versions."""
        checkPythonVersion((3, 10, 0))
        with self.assertRaisesRegex(RuntimeError, "requires Python 3.10 or newer"):
            checkPythonVersion((3, 9, 20))

    def testVersion(self):
        """Test that python=... is working."""
        output = self.runTests("-i", "python_version").output
        self.assertIn("[PYTHON != 2]", output)
        self.assertIn("[PYTHON != 3.5]", output)
        self.assertIn("[PYTHON != 3.4.1]", output)
