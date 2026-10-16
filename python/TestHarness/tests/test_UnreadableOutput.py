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
    def testUnreadableOutput(self):
        """
        Test for bad output supplied by executed commands
        """

        out = self.runTests("-i", "non_unicode", "-v").output
        self.assertRegex(out, r"non_unicode.*INVALID UTF-8 IN STDOUT.*OK")
