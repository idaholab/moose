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
    """
    Test general PBS functionality. There are some caveats however:

    We cannot test the output of specific test. Only the initial launch return code. This
    is because launching qsub is a background process, and we have no idea when that job
    is finished. Or if it even began (perhaps the job is queued).
    """
    def testPBSQueue(self):
        """
        Test argument "--queue-queue does-not-exist" fails, as this queue should not exist
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--queue-queue', 'does-not-exist', '--pbs', 'testPBSQueue', '-i', 'pbs_test')
        e = cm.exception
        self.assertRegexpMatches(e.output, r'ERROR: qsub: Unknown queue')

    def testPBSLaunch(self):
        """
        Test general launch command
        """
        output = self.runTests('--pbs', 'testPBSLaunch', '-i', 'pbs_test')
        self.assertNotIn('LAUNCHED', output)
