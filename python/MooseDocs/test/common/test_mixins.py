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
from MooseDocs.common import exceptions, mixins

class Foo(mixins.ConfigObject):
    """Testing instance of ConfigObject."""
    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['foo'] = ('bar', "Testing...")
        return config

class Bar(mixins.ConfigObject):
    """Testing instance of ConfigObject."""
    @staticmethod
    def defaultConfig():
        return None

class TestConfigObject(unittest.TestCase):
    """
    Test basic use of ConfigObject.
    """

    def testDefaultConfig(self):
        """
        Test defaultConfig returns class level options.
        """
        obj = Foo()
        self.assertIn('foo', obj)

    def testBadDefaultConfigReturn(self):
        """
        Test exception from defaultConfig.
        """
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Bar()
        self.assertIn("The return type from 'defaultConfig'", str(e.exception))

    def testUpdateAndGet(self):
        """
        Test update method.
        """
        obj = Foo()
        obj.update(foo='foo')
        self.assertEqual(obj['foo'], 'foo')
        self.assertEqual(obj.get('foo'), 'foo')
        self.assertIsNone(obj.get('bar', None))

    def testUnknown(self):
        """
        Test unknown config exception.
        """
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Foo(unknown=42)
        self.assertIn("The following config options", str(e.exception))
        self.assertIn("unknown", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
