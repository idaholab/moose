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
from MooseDocs.tree import tokens

Test = tokens.newToken('Test', foo='bar')
class TestTokens(unittest.TestCase):

    def testToken(self):
        token = Test(None)
        self.assertEqual(token.name, 'Test')
        self.assertTrue(token['recursive'])
        self.assertEqual(token['foo'], 'bar')

        token = Test(None, recursive=False, foo='foo')
        self.assertFalse(token['recursive'])
        self.assertEqual(token['foo'], 'foo')

    def testToDict(self):
        token = Test(None)
        self.assertEqual(token.toDict(),
                         {'attributes': {'foo': 'bar', 'recursive': True}, 'children': [], 'name': 'Test'})

if __name__ == '__main__':
    unittest.main(verbosity=2)
