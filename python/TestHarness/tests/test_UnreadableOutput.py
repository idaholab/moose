#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testUnreadableOutput(self):
        """
        Test for bad output supplied by executed commands
        """

        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'non_unicode')
        e = cm.exception
        self.assertIn('non-unicode characters in output', e.output.decode('utf-8'))
