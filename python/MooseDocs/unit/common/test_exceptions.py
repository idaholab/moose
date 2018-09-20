#!/usr/bin/env python2
import unittest
from MooseDocs.common import exceptions

class TestExceptions(unittest.TestCase):
    def testMooseDocsException(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            raise exceptions.MooseDocsException("{}", 'foo')
        self.assertEqual('foo', e.exception.message)

    def testTokenizeException(self):
        with self.assertRaises(exceptions.TokenizeException) as e:
            raise exceptions.TokenizeException("{}", 'foo')
        self.assertEqual('foo', e.exception.message)

    def testRenderException(self):
        with self.assertRaises(exceptions.RenderException) as e:
            raise exceptions.RenderException(42, "{}", 'foo')
        self.assertEqual('foo', e.exception.message)
        self.assertEqual(e.exception.info, 42)

if __name__ == '__main__':
    unittest.main(verbosity=2)
