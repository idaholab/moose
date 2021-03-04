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
import collections
import unittest
import mooseutils
from moosesqa import Requirement, number_requirements, get_requirements_from_file
from moosesqa import  get_requirements_from_tests, get_test_specification
from moosesqa.get_requirements import _find_file

class TestGetRequirementsFromFile(unittest.TestCase):
    def testBasic(self):
        MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        filename = os.path.join(MOOSE_DIR, 'python', 'moosesqa', 'test', 'specs', 'spec_basic')
        req = get_requirements_from_file(filename)

        self.assertEqual(len(req), 2)
        self.assertEqual(req[0].name, 'single')
        self.assertEqual(req[0].filename, filename)
        self.assertEqual(req[0].line, 2)
        self.assertEqual(req[0].label, None)
        self.assertEqual(req[0].requirement, 'A simple requirement')
        self.assertEqual(req[0].requirement_line, 3)
        self.assertEqual(req[0].issues, ['#1234'])
        self.assertEqual(req[0].issues_line, 4)
        self.assertEqual(req[0].design, ['file.md'])
        self.assertEqual(req[0].design_line, 5)
        self.assertEqual(req[0].details, list())
        spec = req[0].specification
        self.assertEqual(spec.name, 'single')
        self.assertEqual(spec.filename, filename)
        self.assertEqual(spec.line, 2)
        self.assertEqual(spec.type, None)
        self.assertEqual(spec.prerequisites, set())
        self.assertEqual(spec.skip, False)
        self.assertEqual(spec.deleted, False)

        self.assertEqual(req[1].name, 'group')
        self.assertEqual(req[1].filename, filename)
        self.assertEqual(req[1].line, 8)
        #self.assertEqual(req[0].specification, None)
        self.assertEqual(req[1].label, None)
        self.assertEqual(req[1].requirement, 'A group requirement')
        self.assertEqual(req[1].requirement_line, 9)
        self.assertEqual(req[1].issues, ['#4321'])
        self.assertEqual(req[1].issues_line, 10)
        self.assertEqual(req[1].design, ['other.md'])
        self.assertEqual(req[1].design_line, 11)
        self.assertEqual(req[1].specification, None)
        self.assertEqual(len(req[1].details), 2)

        d0 = req[1].details[0]
        self.assertEqual(d0.name, 'a')
        self.assertEqual(d0.filename, filename)
        self.assertEqual(d0.line, 12)
        self.assertEqual(d0.detail, 'a')
        self.assertEqual(d0.detail_line, 13)
        spec = d0.specification
        self.assertEqual(spec.name, 'group/a')
        self.assertEqual(spec.filename, filename)
        self.assertEqual(spec.line, 12)
        self.assertEqual(spec.type, None)
        self.assertEqual(spec.prerequisites, set())
        self.assertEqual(spec.skip, False)
        self.assertEqual(spec.deleted, False)

        d1 = req[1].details[1]
        self.assertEqual(d1.name, 'b')
        self.assertEqual(d1.filename, filename)
        self.assertEqual(d1.line, 15)
        self.assertEqual(d1.detail, 'b')
        self.assertEqual(d1.detail_line, 16)
        spec = d1.specification
        self.assertEqual(spec.name, 'group/b')
        self.assertEqual(spec.filename, filename)
        self.assertEqual(spec.line, 15)
        self.assertEqual(spec.type, None)
        self.assertEqual(spec.prerequisites, set())
        self.assertEqual(spec.skip, False)
        self.assertEqual(spec.deleted, False)

    def testError(self):
        with self.assertRaises(FileNotFoundError) as e:
            req = get_requirements_from_file('wrong')
        self.assertIn('The supplied filename does not exist: wrong', str(e.exception))

class TestNumberRequirements(unittest.TestCase):
    def testBase(self):
        r0 = Requirement()
        r1 = Requirement()
        r2 = Requirement()
        req = collections.OrderedDict(a=[r0, r1], b=[r2])

        number_requirements(req, 1980)
        self.assertEqual(r0.label, '1980.1.1')
        self.assertEqual(r1.label, '1980.1.2')
        self.assertEqual(r2.label, '1980.2.1')

class TestFindFile(unittest.TestCase):
    def testExact(self):
        fname = '/test/tests/kernels/simple_diffusion/tests'
        root_dir = mooseutils.git_root_dir(os.path.dirname(__file__))

        filename = _find_file(root_dir, fname)
        self.assertEqual(filename, os.path.join(root_dir, fname.strip('/')))

    def testEndswith(self):
        fname = 'markers/box_marker/tests'
        root_dir = mooseutils.git_root_dir(os.path.dirname(__file__))

        filename = _find_file(root_dir, fname)
        self.assertEqual(filename, os.path.join(root_dir, 'test', 'tests', 'markers', 'box_marker', 'tests'))

    def testNoneError(self):
        fname = 'markers/box_marker/wrong'
        root_dir = mooseutils.git_root_dir(os.path.dirname(__file__))
        with self.assertRaises(NameError) as e:
            filename = _find_file(root_dir, fname)
        self.assertIn('Unable to locate a test specification', str(e.exception))

    def testToManyError(self):
        fname = 'box_marker/tests'
        root_dir = mooseutils.git_root_dir(os.path.dirname(__file__))
        with self.assertRaises(NameError) as e:
            filename = _find_file(root_dir, fname)
        self.assertIn('Located multiple test specifications', str(e.exception))

class TestGetTestSpecification(unittest.TestCase):
    def testBasic(self):
        MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        filename = os.path.join(MOOSE_DIR, 'python', 'moosesqa', 'test', 'specs', 'spec_basic')

        spec = get_test_specification(filename, 'single')
        self.assertEqual(spec.name, 'single')
        self.assertEqual(spec.filename, filename)
        self.assertEqual(spec.line, 2)
        self.assertEqual(spec.type, None)
        self.assertEqual(spec.prerequisites, set())
        self.assertEqual(spec.skip, False)
        self.assertEqual(spec.deleted, False)

        spec = get_test_specification(filename, 'group/a')
        self.assertEqual(spec.name, 'group/a')
        self.assertEqual(spec.filename, filename)
        self.assertEqual(spec.line, 12)
        self.assertEqual(spec.type, None)
        self.assertEqual(spec.prerequisites, set())
        self.assertEqual(spec.skip, False)
        self.assertEqual(spec.deleted, False)

        with self.assertRaises(KeyError) as e:
            spec = get_test_specification(filename, 'wrong')
        self.assertIn("Unable to locate 'wrong'", str(e.exception))

class TestGetRequirementsFromTests(unittest.TestCase):
    def testBasic(self):
        MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        loc = [os.path.join(MOOSE_DIR, 'test', 'tests', 'markers')]
        req = get_requirements_from_tests(loc, ['tests'])

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

        self.assertIn('markers/box_marker/tests', r0.filename)
        self.assertIn('markers/box_marker/tests', r1.filename)

        spec0 = r0.specification
        self.assertEqual(spec0.name, 'mark_only')
        self.assertIn('markers/box_marker/tests', spec0.filename)

        self.assertEqual(r0.prefix, 'box_marker')
        self.assertEqual(r1.prefix, 'box_marker')

    def testRequirementWithDetails(self):
        loc = [os.getcwd()]
        req = get_requirements_from_tests(loc, ['test_get_requirements_spec0'])

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

class TestRequirementCollections(unittest.TestCase):
    def testAvailable(self):
        # Help avoid this being changed without docs being updated
        import moosesqa
        self.assertEqual(moosesqa.MOOSESQA_COLLECTIONS, {'FUNCTIONAL', 'USABILITY', 'PERFORMANCE', 'SYSTEM', 'FAILURE_ANALYSIS'},
                         "If you are adding to moosesqa.MOOSESQA_COLLECTIONS make sure you update associated documentation.")

if __name__ == '__main__':
    unittest.main(verbosity=2)
