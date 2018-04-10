#!/usr/bin/env python2
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
