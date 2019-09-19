#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess, os
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testValidMPI(self):
        """
        Test for a failing mpiexec
        """
        # Kill two birds with one stone. Verify the TestHarness obeys the environment variable MOOSE_MPI_COMMAND,
        # while setting that command to a binary which always returns a non-zero exit code (`false`).
        os.environ['MOOSE_MPI_COMMAND'] = 'false'

        output = self.runTests('-i', 'always_ok', '-p2', '--no-color')
        self.assertRegexpMatches(output, 'tests/test_harness.always_ok.*? \[MPI CHECK FAILED\] SKIP')
