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

import unittest
import chigger
import mock
import subprocess
import os.path

class Test(unittest.TestCase):
    @mock.patch('subprocess.call')
    def testAnimate(self, subproc):
        chigger.utils.animate(os.path.join('..', 'adapt', 'gold', 'adapt_*.png'), 'out.gif')
        subproc.assert_called_with(['convert', '-delay', '20', os.path.join('..', 'adapt', 'gold', 'adapt_0.png'),
                                               '-delay', '20', os.path.join('..', 'adapt', 'gold', 'adapt_4.png'),
                                               '-delay', '500', os.path.join('..', 'adapt', 'gold', 'adapt_9.png'), '-loop', '0', 'out.gif'])

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
