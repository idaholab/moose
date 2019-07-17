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
import unittest
import mooseutils
import pickle

@mooseutils.addProperty('prop')
class MyNode(mooseutils.AutoPropertyMixin):
    """Global for pickle test."""
    pass

class Test(unittest.TestCase):
    """Test mooseutils.AutoPropertyMixin class."""

    def testProperty(self):
        @mooseutils.addProperty('prop')
        class MyNode(mooseutils.AutoPropertyMixin):
            pass

        node = MyNode()
        self.assertTrue(hasattr(node, 'prop'))
        self.assertIsNone(node.prop)
        node.prop = 1
        self.assertEqual(node.prop, 1)
        node.prop = 'foo'
        self.assertEqual(node.prop, 'foo')

        node = MyNode(prop=1)
        self.assertEqual(node.prop, 1)

    def testPropertyRequired(self):

        @mooseutils.addProperty('prop', required=True)
        class MyNode(mooseutils.AutoPropertyMixin):
            pass

        with self.assertRaises(mooseutils.MooseException) as e:
            node = MyNode()
        self.assertIn("The property 'prop' is required.", e.exception.message)

        node = MyNode(prop=1)
        self.assertEqual(node.prop, 1)

    def testPropertyDefault(self):
        @mooseutils.addProperty('prop', default=12345)
        class MyNode(mooseutils.AutoPropertyMixin):
            pass
        node = MyNode()
        self.assertEqual(node.prop, 12345)
        node.prop = 'combo'
        self.assertEqual(node.prop, 'combo')

        node = MyNode(prop=34567)
        self.assertEqual(node.prop, 34567)

    def testPropertyType(self):
        @mooseutils.addProperty('prop', ptype=int)
        class MyNode(mooseutils.AutoPropertyMixin):
            pass

        node = MyNode()
        self.assertIsNone(node.prop)
        with self.assertRaises(mooseutils.MooseException) as e:
            node.prop = 'foo'
        self.assertIn("The supplied property 'prop' must be of type 'int', but 'str' was provided.",
                      e.exception.message)

        node.prop = 12345
        self.assertEqual(node.prop, 12345)

    def testPropertyInheritance(self):

        @mooseutils.addProperty('prop0')
        class N0(mooseutils.AutoPropertyMixin):
            pass

        @mooseutils.addProperty('prop1')
        class N1(N0):
            pass

        n0 = N0(prop0=1)
        self.assertTrue(hasattr(n0, 'prop0'))
        self.assertFalse(hasattr(n0, 'prop1'))
        self.assertEqual(n0.prop0, 1)

        n1 = N1(prop0=2, prop1=3)
        self.assertTrue(hasattr(n1, 'prop0'))
        self.assertTrue(hasattr(n1, 'prop1'))
        self.assertEqual(n1.prop0, 2)
        self.assertEqual(n1.prop1, 3)

    def testAttributes(self):

        @mooseutils.addProperty('bar')
        class N(mooseutils.AutoPropertyMixin):
            pass

        n = N(foo=1)
        self.assertFalse(hasattr(n, 'foo'))
        self.assertIn('foo', n)
        self.assertNotIn('bar', n)
        self.assertEqual(n['foo'], 1)

        n['foo'] = 2
        self.assertEqual(n['foo'], 2)

    def testCustomProperty(self):
        class MyProperty(mooseutils.Property):
            pass
        @mooseutils.addProperty('bar', cls=MyProperty)
        class MyObject(mooseutils.AutoPropertyMixin):
            pass

        obj = MyObject(bar=42)
        self.assertIsInstance(list(obj.__DESCRIPTORS__[MyObject])[0], MyProperty)

    def testPickle(self):
        node = MyNode(prop=12345)
        self.assertEqual(node.prop, 12345)

        filename = 'tmpNodeData.pk1'
        with open(filename, 'w+') as fid:
            pickle.dump(node, fid, -1)
        with open(filename, 'r') as fid:
            node1 = pickle.load(fid)
        os.remove(filename)

        self.assertEqual(node1.prop, 12345)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
