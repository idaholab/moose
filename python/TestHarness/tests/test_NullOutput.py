# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testNullOutput(self):
        """
        Test for NULL characters in the output of executed commands
        """

        out = self.runTests("-i", "null_output", "-v").output
        self.assertRegex(
            out, r"null_output.*INVALID OUTPUT CHARACTERS IN RUNNER_RUN.*OK"
        )
        # NULL characters are kept in the output as a visible marker
        self.assertIn("beforeNULLafter", out)
