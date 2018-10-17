#!/usr/bin/env python2
import unittest
from MooseDocs.common import exceptions

class TestExceptions(unittest.TestCase):
    def testMooseDocsException(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            raise exceptions.MooseDocsException("{}", 'foo')
        self.assertEqual('foo', e.exception.message)

if __name__ == '__main__':
    unittest.main(verbosity=2)
