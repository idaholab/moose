#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest

import MooseDocs
from MooseDocs import common

class TestGetRequirements(unittest.TestCase):
    def testBasic(self):
        loc = [os.path.join(MooseDocs.MOOSE_DIR, 'test', 'tests', 'markers')]
        req = common.get_requirements(loc, ['tests'])

        r0 = req['box_marker'][0]
        r1 = req['box_marker'][1]

        self.assertEqual(r0.issues, ['#1275'])
        self.assertEqual(r1.issues, ['#1275'])

        self.assertEqual(r0.design, ['/Markers/index.md', '/BoxMarker.md'])
        self.assertEqual(r1.design, ['/Markers/index.md', '/BoxMarker.md'])

        self.assertIn('create an aux', r0.text)
        self.assertIn('within a rec', r1.text)

if __name__ == '__main__':
    unittest.main(verbosity=2)
