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
from moosesqa import get_requirements

class TestGetRequirements(unittest.TestCase):
    def testBasic(self):
        MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        loc = [os.path.join(MOOSE_DIR, 'test', 'tests', 'markers')]
        req = get_requirements(loc, ['tests'])

        r0 = req['box_marker'][0]
        r1 = req['box_marker'][1]

        self.assertEqual(r0.issues, ['#1275'])
        self.assertEqual(r1.issues, ['#1275'])

        self.assertEqual(r0.design, ['/Markers/index.md', '/BoxMarker.md'])
        self.assertEqual(r1.design, ['/Markers/index.md', '/BoxMarker.md'])

        self.assertIn('create an aux', r0.requirement)
        self.assertIn('within a rec', r1.requirement)

        self.assertEqual(r0.name, 'mark_only')
        self.assertEqual(r1.name, 'mark_and_adapt')

        self.assertEqual(r0.path, 'box_marker')
        self.assertEqual(r1.path, 'box_marker')

        self.assertIn('markers/box_marker/tests', r0.filename)
        self.assertIn('markers/box_marker/tests', r1.filename)

        spec0 = r0.specification
        self.assertEqual(spec0.name, 'mark_only')
        self.assertEqual(spec0.path, 'box_marker')
        self.assertIn('markers/box_marker/tests', spec0.filename)

    def testRequirementWithDetails(self):
        loc = [os.getcwd()]
        req = get_requirements(loc, ['test_get_requirements_spec0'])

        self.assertEqual(len(req), 1)
        self.assertEqual(len(req['test_get_requirements_spec0']), 19)

        r = req['test_get_requirements_spec0'][0]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.requirement, "Requirement One")

        r = req['test_get_requirements_spec0'][1]
        self.assertEqual(r.issues, ['#3456'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.requirement, "Requirement Two")

        r = req['test_get_requirements_spec0'][2]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['bibtex.md'])
        self.assertEqual(r.requirement, "Requirement Three")

        r = req['test_get_requirements_spec0'][3]
        self.assertEqual(r.issues, ['#4567'])
        self.assertEqual(r.design, ['katex.md'])
        self.assertEqual(r.requirement, "Requirement Four")

        r = req['test_get_requirements_spec0'][4]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.requirement, "Requirement Group One")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group0-a')
        self.assertEqual(d.detail, '1D')
        self.assertEqual(d.detail_line, 27)

        d = r.details[1]
        self.assertEqual(d.name, 'group0-b')
        self.assertEqual(d.detail, '2D')
        self.assertEqual(d.detail_line, 30)

        r = req['test_get_requirements_spec0'][5]
        self.assertEqual(r.issues, ['#8910'])
        self.assertEqual(r.design, ['core.md'])
        self.assertEqual(r.requirement, "Requirement Group Two")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group1-a')
        self.assertEqual(d.detail, '3D')
        self.assertEqual(d.detail_line, 38)

        d = r.details[1]
        self.assertEqual(d.name, 'group1-b')
        self.assertEqual(d.detail, '4D')
        self.assertEqual(d.detail_line, 41)

        r = req['test_get_requirements_spec0'][6]
        self.assertEqual(r.issues, ['#1234'])
        self.assertEqual(r.design, ['bibtex.md'])
        self.assertEqual(r.requirement, "Requirement Group Three")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group2-a')
        self.assertEqual(d.detail, '5D')
        self.assertEqual(d.detail_line, 50)

        d = r.details[1]
        self.assertEqual(d.name, 'group2-b')
        self.assertEqual(d.detail, '6D')
        self.assertEqual(d.detail_line, 53)

        r = req['test_get_requirements_spec0'][7]
        self.assertEqual(r.issues, ['#4321'])
        self.assertEqual(r.design, ['katex.md'])
        self.assertEqual(r.requirement, "Requirement Group Four")
        self.assertEqual(len(r.details), 2)

        d = r.details[0]
        self.assertEqual(d.name, 'group3-a')
        self.assertEqual(d.detail, '7D')
        self.assertEqual(d.detail_line, 62)

        d = r.details[1]
        self.assertEqual(d.name, 'group3-b')
        self.assertEqual(d.detail, '8D')
        self.assertEqual(d.detail_line, 65)

        # collections_default
        r = req['test_get_requirements_spec0'][9]
        self.assertIsNone(r.collections)

        # collections_override
        r = req['test_get_requirements_spec0'][10]
        self.assertEqual(r.collections, {'A'})

        # collections_group
        r = req['test_get_requirements_spec0'][11]
        self.assertEqual(r.collections, {'A', 'B'})

        # types
        r = req['test_get_requirements_spec0'][12]
        self.assertEqual(r.types, {'A'})

        # types_group
        r = req['test_get_requirements_spec0'][13]
        self.assertEqual(r.types, {'A', 'B'})

        # names
        r = req['test_get_requirements_spec0'][14]
        self.assertEqual(r.names, {'names'})

        # names_group
        r = req['test_get_requirements_spec0'][15]
        self.assertEqual(r.names, {'names_group/a', 'names_group/b'})

        # prereq_first
        r = req['test_get_requirements_spec0'][16]
        self.assertEqual(r.prerequisites, set())

        # prereq_first
        r = req['test_get_requirements_spec0'][17]
        self.assertEqual(r.prerequisites, {'prereq_first'})

        # prereq_group
        r = req['test_get_requirements_spec0'][18]
        self.assertEqual(r.prerequisites, {'prereq_first', 'prereq_group/a'})

if __name__ == '__main__':
    unittest.main(verbosity=2)
