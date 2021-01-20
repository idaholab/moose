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
import os
import json
from FactorySystem import MooseObject, Factory

class TestObject(MooseObject):
    @staticmethod
    def validParams():
        params = MooseObject.validParams()
        params.addParam("year", 1980, "The best year")
        return params

class FactoryTester(unittest.TestCase):
    def setUp(self):
        fname = 'testoutput.json'
        if os.path.isfile(fname):
            os.remove(fname)

    def testJSON(self):
        gold = {'TestObject': {'year': {'default': 1980, 'description': 'The best year'}}}

        factory = Factory()
        factory.register(TestObject, 'TestObject')
        out = factory.dumpJSON(None)
        self.assertEqual(out, gold)
        self.assertFalse(os.path.isfile('testoutput.json'))

        out = factory.dumpJSON()
        self.assertTrue(os.path.isfile('testoutput.json'))
        with open('testoutput.json', 'r') as f:
            data = json.load(f)
        self.assertEqual(data, gold)
        self.assertEqual(out, gold)

if __name__ == '__main__':
    unittest.main(verbosity=2)
