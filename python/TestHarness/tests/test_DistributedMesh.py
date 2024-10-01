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
    def testSyntax(self):
        """
        Test for correct operation with distributed mesh tests
        """

        # Verify the distributed mesh test is skipped
        output = self.runTests('-i', 'mesh_mode_distributed', '--no-color')
        self.assertIn('[MESH_MODE!=DISTRIBUTED] SKIP', output)

        # Verify the distributed mesh test is passing when providing --distributed
        # To be acurate, test for OK rather than asserting if 'distributed' is
        # missing from the output.
        output = self.runTests('--distributed', '-i', 'mesh_mode_distributed')
        self.assertRegex(output, 'test_harness.distributed_mesh.*?OK')
