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
import multiprocessing

@mooseutils.addProperty('prop')
class MyNode(mooseutils.AutoPropertyMixin):
    """Global for pickle test."""
    pass

@mooseutils.addProperty('prop', required=True)
class MyNode2(mooseutils.AutoPropertyMixin):
    """Global for pickle test, with required property."""
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
        with open(filename, 'wb') as fid:
            pickle.dump(node, fid, protocol=pickle.HIGHEST_PROTOCOL)
        with open(filename, 'rb') as fid:
            node1 = pickle.load(fid)
        os.remove(filename)

        self.assertEqual(node1.prop, 12345)

    def testPickleRequired(self):
        node = MyNode2(prop=12345)
        self.assertEqual(node.prop, 12345)

        filename = 'tmpNodeData.pk1'
        with open(filename, 'wb') as fid:
            pickle.dump(node, fid, protocol=pickle.HIGHEST_PROTOCOL)
        with open(filename, 'rb') as fid:
            node1 = pickle.load(fid)
        os.remove(filename)

        self.assertEqual(node1.prop, 12345)

    def testMutable(self):
        node = MyNode(False, year=1949, prop=12345)
        with self.assertRaises(mooseutils.MooseException) as e:
            node['year'] = 1980
        self.assertIn("The MyNode object is immutable", e.exception.message)

        with self.assertRaises(mooseutils.MooseException) as e:
            node.prop = 42
        self.assertIn("The MyNode object is immutable", e.exception.message)

        with self.assertRaises(mooseutils.MooseException) as e:
            node.attributes
        self.assertIn("The MyNode object is immutable", e.exception.message)

        node._AutoPropertyMixin__mutable = True
        node['year'] = 1980
        self.assertEqual(node['year'], 1980)
        node.prop = 42
        self.assertEqual(node.prop, 42)
        self.assertIsInstance(node.attributes, dict)

    def testParallel(self):

        @mooseutils.addProperty('uid')
        class MyNode(mooseutils.AutoPropertyMixin):
            pass

        manager = multiprocessing.Manager()
        page_attributes = manager.dict()

        page = MyNode(uid=0)
        p = multiprocessing.Process(target=self._addAttribute, args=(page, 1949, page_attributes))
        p.start()
        p.join()

        self.assertNotIn('year', page.attributes)
        self.assertIn(page.uid, page_attributes)
        self.assertEqual(page_attributes[page.uid], dict(year=1949))
        page.update(page_attributes[page.uid])
        self.assertIn('year', page.attributes)
        self.assertEqual(page['year'], 1949)

        p = multiprocessing.Process(target=self._addAttribute, args=(page, 1980, page_attributes))
        p.start()
        p.join()

        self.assertIn(page.uid, page_attributes)
        self.assertEqual(page_attributes[page.uid], dict(year=1980))
        self.assertEqual(page['year'], 1949)
        page.update(page_attributes[page.uid])
        self.assertEqual(page['year'], 1980)

    @staticmethod
    def _addAttribute(node, year, page_attributes):
        node['year'] = year
        page_attributes[node.uid] = node.attributes

    def testParallelBarrier(self):

        @mooseutils.addProperty('uid', 42)
        class MyNode(mooseutils.AutoPropertyMixin):
            pass

        n0 = MyNode(uid=0)
        n1 = MyNode(uid=1)
        self._pages = [n0, n1]


        barrier = multiprocessing.Barrier(2)
        manager = multiprocessing.Manager()
        page_attributes = manager.dict()

        p0 = multiprocessing.Process(target=self._addAttributeBarrier, args=(n0, barrier, page_attributes))
        p0.start()

        p1 = multiprocessing.Process(target=self._addAttributeBarrier, args=(n1, barrier, page_attributes))
        p1.start()

        p0.join()
        p1.join()

        self.assertEqual(page_attributes[0]['year'], 1949)
        self.assertEqual(page_attributes[1]['year'], 1980)


    def _addAttributeBarrier(self, node, barrier, page_attributes):

        node['year'] = 1949 if node.uid == 0 else 1980
        page_attributes[node.uid] = node.attributes

        barrier.wait()
        self._pages[0].update(page_attributes[0])
        self._pages[1].update(page_attributes[1])

        self.assertEqual(self._pages[0]['year'], 1949)
        self.assertEqual(self._pages[1]['year'], 1980)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
