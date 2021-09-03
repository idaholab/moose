#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
import re

class TestHarnessTester(TestHarnessTestCase):
    def testExtraInfo(self):
        """
        Test for the presence of -e (Extra Info) caveats when
        requested.

        Note: This test is really not demonstrating any of the
        special 'checks' contained within it. We are simply
        testing the TestHarness's ability to print that caveat.

        Because the TestHarness joins all caveats, this is going
        to run as one test, in which we will regexp through a huge
        single caveat line.
        """

        # All the caveats we will verify that should exist in the
        # output
        caveats = ['ASIO', 'DTK', 'UNIQUE_IDS', 'CXX11', 'SUPERLU',
                   'DOF_ID_BYTES', 'TECPLOT', 'PETSC_VERSION_RELEASE',
                   'SLEPC_VERSION', 'MESH_MODE', 'METHOD', 'BOOST',
                   'PETSC_DEBUG', 'LIBRARY_MODE', 'PETSC_VERSION',
                   'CURL', 'THREADING', 'SLEPC', 'VTK', 'UNIQUE_ID',
                   'COMPILER', 'FPARSER_JIT', 'PARMETIS', 'CHACO',
                   'PARTY', 'PTSCOTCH', 'EXODUS_VERSION']

        # Verify all special TestHarness 'checks' are printed. We
        # will use the --ignore feature to force the test to run
        # regardless if that check(s) would otherwise cause this
        # test to be skipped.
        output = self.runTests('-c', '-i', 'extra_info', '--ignore', '-e').decode('utf-8')

        # Parse the output, and find the caveat string
        raw_caveat_string = re.findall(r'\[(.*)\]', output)
        output_caveats = raw_caveat_string[0].split(',')

        # Do the comparison and assert if different. Using a
        # differential set grants us the benifit of forcing us to
        # keep this unittest up to date.
        self.assertEqual(set(caveats), set(output_caveats))
