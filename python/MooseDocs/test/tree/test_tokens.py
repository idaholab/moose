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

    def testCopyToToken(self):
        from MooseDocs.extensions import core

        token = Test(None)
        core.Word(token, content=u'Word')
        core.Space(token, count=1)
        core.Word(token, content=u'Word')

        t2 = Test(None)
        token.copyToToken(t2)

        self.assertEqual(t2(0).name, u'Word')
        self.assertEqual(t2(0)['content'], u'Word')

        self.assertEqual(t2(1).name, u'Space')
        self.assertEqual(t2(1)['count'], 1)

        self.assertEqual(t2(2).name, u'Word')
        self.assertEqual(t2(2)['content'], u'Word')



if __name__ == '__main__':
    unittest.main(verbosity=2)
