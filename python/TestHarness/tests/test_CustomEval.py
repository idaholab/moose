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
  def testCustomEval(self):
    out = self.runTests('-i', 'custom_eval', exit_code=128).output
    self.assertIn('Custom evaluation failed', out)

    #test expect out failure
    out = self.runTests('-i', 'custom_eval', exit_code=128).output
    self.assertIn('expect_out and absent_out can not be supplied', out)
