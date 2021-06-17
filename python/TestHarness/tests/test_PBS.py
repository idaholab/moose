#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess, unittest, os
from TestHarnessTestCase import TestHarnessTestCase

def checkQstat():
    try:
        if subprocess.call(['qstat']) == 0:
            return True
    except:
        pass

@unittest.skipIf(checkQstat() != True, "PBS not available")
class TestHarnessTester(TestHarnessTestCase):
    """
    Test general PBS functionality. There are some caveats however:

    We cannot test the output of specific test. Only the initial launch return code. This
    is because launching qsub is a background process, and we have no idea when that job
    is finished. Or if it even began (perhaps the job is queued).
    """
    def setUp(self):
        """
        setUp occurs before every test. Clean up previous results file
        """
        pbs_results_file = os.path.join(os.getenv('MOOSE_DIR'), 'test', '_testPBS')

        # File will not exist on the first run
        try:
            os.remove(pbs_results_file)
        except:
            pass

    def testPBSQueue(self):
        """
        Test argument '--pbs-queue does-not-exist' fails, as this queue should not exist
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--pbs-queue', 'does-not-exist', '--pbs', '_testPBS', '-i', 'always_ok')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'ERROR: qsub: Unknown queue')

    def testPBSLaunch(self):
        """
        Test general launch command
        """
        output = self.runTests('--pbs', '_testPBS', '-i', 'always_ok').decode('utf-8')
        self.assertNotIn('LAUNCHED', output)
