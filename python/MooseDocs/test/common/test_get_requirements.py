#!/usr/bin/env python3
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
import logging

import MooseDocs
from MooseDocs import common

logging.basicConfig()

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

        self.assertEqual(r0.name, 'mark_only')
        self.assertEqual(r1.name, 'mark_and_adapt')

        self.assertEqual(r0.path, 'box_marker')
        self.assertEqual(r1.path, 'box_marker')

        self.assertIn('markers/box_marker/tests', r0.filename)
        self.assertIn('markers/box_marker/tests', r1.filename)

    def testRequirementWithDetails(self):
        loc = [os.getcwd()]
        req = common.get_requirements(loc, ['demo'])

        self.assertEqual(len(req), 1)
        self.assertEqual(len(req['demo']), 8)

        r = req['demo'][0]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.text, "Requirement One")

        r = req['demo'][1]
        self.assertEqual(r.issues, ['#3456'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.text, "Requirement Two")

        r = req['demo'][2]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['bibtex.md'])
        self.assertEqual(r.text, "Requirement Three")

        r = req['demo'][3]
        self.assertEqual(r.issues, ['#4567'])
        self.assertEqual(r.design, ['katex.md'])
        self.assertEqual(r.text, "Requirement Four")

        r = req['demo'][4]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.text, "Requirement Group One")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group0-a')
        self.assertEqual(d.text, '1D')
        self.assertEqual(d.text_line, 27)

        d = r.details[1]
        self.assertEqual(d.name, 'group0-b')
        self.assertEqual(d.text, '2D')
        self.assertEqual(d.text_line, 30)

        r = req['demo'][5]
        self.assertEqual(r.issues, ['#8910'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.text, "Requirement Group Two")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group1-a')
        self.assertEqual(d.text, '3D')
        self.assertEqual(d.text_line, 38)

        d = r.details[1]
        self.assertEqual(d.name, 'group1-b')
        self.assertEqual(d.text, '4D')
        self.assertEqual(d.text_line, 41)

        r = req['demo'][6]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['bibtex.md'])
        self.assertEqual(r.text, "Requirement Group Three")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group2-a')
        self.assertEqual(d.text, '5D')
        self.assertEqual(d.text_line, 50)

        d = r.details[1]
        self.assertEqual(d.name, 'group2-b')
        self.assertEqual(d.text, '6D')
        self.assertEqual(d.text_line, 53)

        r = req['demo'][7]
        self.assertEqual(r.issues, ['#4321'])
        self.assertEqual(r.design, ['katex.md'])
        self.assertEqual(r.text, "Requirement Group Four")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group3-a')
        self.assertEqual(d.text, '7D')
        self.assertEqual(d.text_line, 62)

        d = r.details[1]
        self.assertEqual(d.name, 'group3-b')
        self.assertEqual(d.text, '8D')
        self.assertEqual(d.text_line, 65)

    def testRequirementWithDeprecated(self):
        loc = [os.getcwd()]
        req = common.get_requirements(loc, ['demo2'])
        self.assertEqual(len(req), 0)
        # TODO: In Python3 performing assert on log messages is possible, it is a pain in 2

if __name__ == '__main__':
    unittest.main(verbosity=2)
