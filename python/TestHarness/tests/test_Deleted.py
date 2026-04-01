# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test deleted tests."""

from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    """Test deleted tests."""

    def testNoRun(self):
        """Test that deleted tests do not run."""
        output = self.runTests("--no-color", "-i", "deleted").output
        self.assertNotIn("tests/test_harness.deleted", output)
