#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testGlobalCapabilitiesPropagated(self):
        result = self.runTests('-i', 'global_capabilities', '--dry-run')
        test_results = result.results['tests']['tests/test_harness']['tests']['always_ok']
        self.assertEqual(test_results['capabilities'], 'foo')

    def testGlobalPrereqPropagated(self):
        result = self.runTests('-i', 'global_prereq', '--dry-run')
        test_results = result.results['tests']['tests/test_harness']['tests']['parent']['tests']['child']
        self.assertEqual(test_results['prereq'], ['pre'])
