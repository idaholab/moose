# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testGlobalCapabilitiesPropagated(self):
        # The capability set on the parent block reaches the child, where "foo" is unknown
        result = self.runTests(
            "-i", "global_capabilities", minimal_capabilities=True, exit_code=132
        )
        assert result.harness is not None
        test = self.getJobWithName(result.harness, "always_ok")
        self.assertEqual(test.getTester().getStatusMessage(), "UNKNOWN CAPABILITIES")
        self.assertIn(
            "The following capabilities are unknown: foo", test.getTester().getOutput()
        )

    def testGlobalPrereqPropagated(self):
        # The parent prereq also reaches "pre" itself, which then depends on itself
        result = self.runTests("-i", "global_prereq", "--dry-run", exit_code=132)
        self.assertRegex(
            result.output,
            r"test_harness\.parent/pre.*FAILED \(unknown dependency pre\)",
        )
