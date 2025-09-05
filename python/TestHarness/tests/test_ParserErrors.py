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
    def testSyntax(self):
        """
        Test for Parser Errors
        """

        # check that parser errors print correctly
        # TODO: Are there more we can test?
        output = self.runTests("-i", "parse_errors", exit_code=128).output
        self.assertIn("duplicate parameter", output)
