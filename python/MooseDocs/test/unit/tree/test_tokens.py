#!/usr/bin/env python2
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


if __name__ == '__main__':
    unittest.main(verbosity=2)
