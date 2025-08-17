#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
import json

class TestHarnessTester(TestHarnessTestCase):
    def testGlobalCapabilitiesPropagated(self):
        result = self.runTests('-i', 'global_capabilities', '--dry-run')
        self.assertTrue("[NEEDS: FOO]" in result.output)

    def testGlobalPrereqPropagated(self):
        try:
            self.runTests('-i', 'global_prereq', '--dry-run')
        except:
            self.assertTrue(True)
            return
        self.assertTrue(False)
