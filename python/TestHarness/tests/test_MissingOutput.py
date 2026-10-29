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
    def testMissingOutput(self):
        """
        Test for an Exodiff output file that the run did not produce
        """
        out = self.runTests("-i", "missing_output", exit_code=128).output
        self.assertRegex(out, r"test_harness\.exodiff.*?FAILED \(MISSING OUTPUT FILE\)")
        self.assertNotRegex(out, r"Running exodiff")
