#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import subprocess
import unittest

class TestChiggerCommandUtility(unittest.TestCase):
    """
    Test the chigger command line utility.
    """
    def execute(self, *args):
        """
        Execute the chigger command line utility with the provided arguments.
        """
        cmd = [os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..',
                                            'scripts', 'chigger'))]
        cmd += list(args)
        return subprocess.check_output(cmd, encoding="utf-8")

    def testInfo(self):
        """
        Test the 'info' command for displaying ExodusII file information.
        """
        out = self.execute('info', os.path.join('..', 'input', 'mug_blocks_out.e'))
        self.assertIn('convected', out)
        self.assertIn('aux_elem', out)
        self.assertIn('func_pp', out)

    def testImg2Mov(self):
        """
        Test 'img2mov' command.
        """
        pattern = os.path.join('..', 'field_data', 'gold', 'plot_current_*.png')
        out = self.execute('img2mov', pattern, '-o', 'output.mov', '--dry-run', '-j', '4', '--duration', '30')
        gold = 'ffmpeg -pattern_type glob -framerate 0 -i ../field_data/gold/plot_current_*.png ' \
               '-b:v 10M -pix_fmt yuv420p -q:v 1 -threads 4 -framerate 0 output.mov'
        self.assertIn(gold, out)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
