#!/usr/bin/env python
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
from moosesqa import Requirement

class TestRequirement(unittest.TestCase):
    def testInit(self):
        kwargs = dict(name='name', path='path', filename='filename', line=1, label='label',
                      requirement='requirement', requirement_line=2,
                      design=['design'], design_line=3,
                      issues=['issues'], issues_line=4,
                      details=['d0', 'd1'], detail='detail', detail_line=5,
                      deprecated=False, deprecated_line=6, duplicate=False,
                      verification=['v0', 'v1'], verification_line=7,
                      validation=['v2', 'v3'], validation_line=8,
                      prerequisites=set(['p0', 'p1']), skip=False, deleted=False)

        req = Requirement(**kwargs)
        for key, value in kwargs.items():
            attrib = getattr(req, key)
            self.assertEqual(attrib, value)

    def testTestable(self):
        req = Requirement()
        self.assertTrue(req.testable)

        req = Requirement(skip=True)
        self.assertFalse(req.testable)

        req = Requirement(deleted=True)
        self.assertFalse(req.testable)

        req = Requirement()
        req.details = [Requirement()]
        self.assertTrue(req.testable)

        req = Requirement()
        req.details = [Requirement(skip=True)]
        self.assertFalse(req.testable)

        req = Requirement()
        req.details = [Requirement(deleted=True)]
        self.assertFalse(req.testable)

    def testStr(self):
        s = str(Requirement(requirement='requirement', design=['design'], issues=['issues']))
        self.assertIn("Requirement: requirement", s)
        self.assertIn("Design: ['design']", s)
        self.assertIn("Issues: ['issues']", s)

if __name__ == '__main__':
    unittest.main(verbosity=2)
