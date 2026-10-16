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
        # Invalid bytes are kept in the output as escapes
        self.assertIn("before\\xff\\xfeafter", out)
        # Each invalid sequence is reported with its location
        self.assertIn("Begin invalid UTF-8 in stdout", out)
        self.assertRegex(out, r"offset 6 of \d+: invalid start byte: b'\\xff'")
        self.assertRegex(out, r"offset 7 of \d+: invalid start byte: b'\\xfe'")
