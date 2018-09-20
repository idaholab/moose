#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testTrimOutput(self):
        """
        Verify output exceeded buffer, and is therfore trimmed
        """
        output = self.runTests('--no-color', '-i', 'trimmed_output', '-v')
        self.assertNotIn('Ouput trimmed', output)
