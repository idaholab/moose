#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import unittest
import mock
import logging
from moosesqa import Requirement, Detail, TestSpecification

class TestTestSpecification(unittest.TestCase):
    def testInit(self):
        kwargs = dict(name='name', filename='filename', line=1, type='Exodiff',
                      prerequisites=set(['p0', 'p1']), skip=False, deleted=False)

        spec = TestSpecification(**kwargs)
        for key, value in kwargs.items():
            attrib = getattr(spec, key)
            self.assertEqual(attrib, value)

    def testTestable(self):
        req = TestSpecification()
        self.assertTrue(req.testable)

        req = TestSpecification(skip=True)
        self.assertFalse(req.testable)

        req = TestSpecification(deleted=True)
        self.assertFalse(req.testable)

class TestRequirement(unittest.TestCase):
    def testInit(self):
        kwargs = dict(name='name', filename='filename', line=1, label='label',
                      requirement='requirement', requirement_line=2,
                      design=['design'], design_line=3,
                      issues=['issues'], issues_line=4,
                      deprecated=False, deprecated_line=6, duplicate=False,
                      verification=['v0', 'v1'], verification_line=7,
                      validation=['v2', 'v3'], validation_line=8)

        req = Requirement(**kwargs)
        for key, value in kwargs.items():
            attrib = getattr(req, key)
            self.assertEqual(attrib, value)

    def testStr(self):
        s = str(Requirement(requirement='requirement', design=['design'], issues=['issues']))
        self.assertIn("requirement = 'requirement'", s)
        self.assertIn("design = ['design']", s)
        self.assertIn("issues = ['issues']", s)

    def testTestable(self):
        req = Requirement()
        self.assertFalse(req.testable)

        req.specification = TestSpecification()
        self.assertTrue(req.testable)

        req.specification.skip = True
        self.assertFalse(req.testable)

        req.specification = None
        self.assertFalse(req.testable)
        req.details.append(Detail(specification=TestSpecification()))
        self.assertTrue(req.testable)

        req.details[0].specification.skip = True
        self.assertFalse(req.testable)

    def testPrerequisites(self):
        req = Requirement()
        self.assertEqual(len(req.prerequisites), 0)

        req = Requirement(specification=TestSpecification(prerequisites={'andrew'}))
        self.assertEqual(len(req.prerequisites), 1)
        self.assertEqual(req.prerequisites, {'andrew'})

        req = Requirement(details=[Detail(specification=TestSpecification(prerequisites={'andrew'})),
                                   Detail(specification=TestSpecification(prerequisites={'1980'}))])
        self.assertEqual(len(req.prerequisites), 2)
        self.assertEqual(req.prerequisites, {'andrew', '1980'})

    def testTypes(self):
        req = Requirement()
        self.assertIsNone(req.types)

        req = Requirement(specification=TestSpecification(type='andrew'))
        self.assertEqual(len(req.types), 1)
        self.assertEqual(req.types, {'andrew'})

        req = Requirement(details=[Detail(specification=TestSpecification(type='andrew')),
                                   Detail(specification=TestSpecification(type='1980'))])
        self.assertEqual(len(req.types), 2)
        self.assertEqual(req.types, {'andrew', '1980'})

    def testCollections(self):
        req = Requirement()
        self.assertIsNone(req.collections)

        req = Requirement(collections={'andrew'})
        self.assertEqual(len(req.collections), 1)
        self.assertEqual(req.collections, {'andrew'})

    def testClassification(self):
        req = Requirement()
        self.assertIsNone(req.classification)

        req = Requirement(classification='andrew')
        self.assertEqual(req.classification, 'andrew')

    def testNames(self):
        req = Requirement()
        self.assertEqual(len(req.names), 0)

        req = Requirement(specification=TestSpecification(name='andrew'))
        self.assertEqual(len(req.names), 1)
        self.assertEqual(req.names, {'andrew'})

        req = Requirement(details=[Detail(specification=TestSpecification(name='andrew')),
                                   Detail(specification=TestSpecification(name='1980'))])
        self.assertEqual(len(req.names), 2)
        self.assertEqual(req.names, {'andrew', '1980'})

class TestDetail(unittest.TestCase):
    def testInit(self):
        kwargs = dict(name='name', filename='filename', line=1)

        spec = Detail(**kwargs)
        for key, value in kwargs.items():
            attrib = getattr(spec, key)
            self.assertEqual(attrib, value)

    def testTestable(self):
        req = Detail()
        self.assertFalse(req.testable)

        req.specification = TestSpecification()
        self.assertTrue(req.testable)

        req.specification.skip = True
        self.assertFalse(req.testable)

if __name__ == '__main__':
    unittest.main(verbosity=2)
