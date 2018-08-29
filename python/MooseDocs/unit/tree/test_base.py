#!/usr/bin/env python2
#pylint: disable=missing-docstring
import unittest
import mock

from MooseDocs.common import exceptions
from MooseDocs.tree import base

class TestNodeBase(unittest.TestCase):
    """
    Tests for NodeBase class.
    """

    def testRoot(self):
        node = base.NodeBase(None)
        self.assertEqual(node.parent, None)

    def testTree(self):
        root = base.NodeBase(None)
        node = base.NodeBase(root)
        self.assertIs(node.parent, root)
        self.assertIs(root.children[0], node)
        self.assertIs(root(0), node)

    @mock.patch('logging.Logger.error')
    def testCallError(self, mock):
        node = base.NodeBase(None)
        node(0)
        mock.assert_called_once()
        args, _ = mock.call_args
        self.assertIn('A child node with index', args[0])

    def testWrite(self):
        node = base.NodeBase(None)
        class TestNode(base.NodeBase):
            def write(self):
                return 'foo'
        TestNode(node)
        TestNode(node)
        self.assertEqual(node.write(), 'foofoo')

    def testIter(self):
        root = base.NodeBase(None)
        child0 = base.NodeBase(root)
        child1 = base.NodeBase(root)
        self.assertEqual(list(root), [child0, child1])

    def testName(self):
        node = base.NodeBase(None, name='test')
        self.assertEqual(node.name, 'test')

        node = base.NodeBase(None)
        self.assertEqual(node.name, 'NodeBase')

    @unittest.skip("Needs work")
    def testParent(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            base.NodeBase(parent=42)
        gold = "The supplied parent must be a NodeBase object, but 'int' was provided."
        self.assertEqual(e.exception.message, gold)

class TestProperty(unittest.TestCase):
    """
    Tests for base.Property() class.
    """
    def testBasic(self):
        prop = base.Property('foo')
        self.assertEqual(prop.name, 'foo')
        self.assertEqual(prop.default, None)
        self.assertEqual(prop.type, None)
        self.assertEqual(prop.required, False)

    def testDefault(self):
        prop = base.Property('foo', 42)
        self.assertEqual(prop.name, 'foo')
        self.assertEqual(prop.default, 42)
        self.assertEqual(prop.type, None)
        self.assertEqual(prop.required, False)

    def testType(self):
        prop = base.Property('foo', 42, int)
        self.assertEqual(prop.name, 'foo')
        self.assertEqual(prop.default, 42)
        self.assertEqual(prop.type, int)
        self.assertEqual(prop.required, False)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            base.Property('foo', ptype='int')
        gold = "The supplied property type (ptype) must be of type 'type', but 'str' provided."
        self.assertEqual(e.exception.message, gold)

    def testRequired(self):
        prop = base.Property('foo', 42, int, True)
        self.assertEqual(prop.name, 'foo')
        self.assertEqual(prop.default, 42)
        self.assertEqual(prop.type, int)
        self.assertEqual(prop.required, True)

    def testKeyword(self):
        prop = base.Property('foo', required=True, default=42, ptype=int)
        self.assertEqual(prop.name, 'foo')
        self.assertEqual(prop.default, 42)
        self.assertEqual(prop.type, int)
        self.assertEqual(prop.required, True)

    def testConstructException(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            base.Property('foo', 42, str)
        self.assertIn("must be of type 'str'", e.exception.message)

class TestNodeBaseWithProperties(unittest.TestCase):
    def testProperties(self):

        class Date(base.NodeBase):
            PROPERTIES = [base.Property('month'), base.Property('year', 1980), base.Property('day', 24, int)]

        # Construction and defaults
        node = Date()
        self.assertTrue(hasattr(node, 'year'))
        self.assertEqual(node.year, 1980)
        self.assertTrue(hasattr(node, 'month'))
        self.assertEqual(node.month, None)
        self.assertTrue(hasattr(node, 'day'))
        self.assertEqual(node.day, 24)

        # Change properties
        node.day = 27
        self.assertEqual(node.day, 27)
        node.year = 1949
        self.assertEqual(node.year, 1949)
        node.month = 'august' # change type allowed because it was not set on construction
        self.assertEqual(node.month, 'august')

        # Set error
        with self.assertRaises(exceptions.MooseDocsException) as e:
            node.day = 1.1
        self.assertIn("must be of type 'int'", e.exception.message)

        # Set properties with kwargs
        node = Date(year=1998)
        self.assertEqual(node.year, 1998)

    def testPropertiesWithMultipleInstances(self):

        class Date(base.NodeBase):
            PROPERTIES = [base.Property('month'), base.Property('year', 1980), base.Property('day', 24, int)]

        node0 = Date(year=1980)
        node1 = Date(year=1981)

        self.assertEqual(node0.year, 1980)
        self.assertEqual(node1.year, 1981)

    def testPropertiesWithKwargs(self):
        class Time(base.NodeBase):
            PROPERTIES = [base.Property('hour', ptype=int), base.Property('minute')]

        t = Time(hour=6)
        self.assertEqual(t.hour, 6)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            Time(hour='str')
        self.assertIn("must be of type 'int'", e.exception.message)

    def testPropertiesNone(self):
        class Time(base.NodeBase):
            PROPERTIES = [base.Property('hour', default=24)]

        t = Time(hour=None)
        self.assertEqual(t.hour, 24)

    def testPropertiesRequired(self):
        class Time(base.NodeBase):
            PROPERTIES = [base.Property('hour', required=True)]

        with self.assertRaises(exceptions.MooseDocsException) as e:
            Time()
        self.assertIn("The property 'hour' is required.", e.exception.message)

    def testAttributes(self):
        class Time(base.NodeBase):
            PROPERTIES = [base.Property('hour', required=True)]

        t = Time(hour=12, minute_=24)
        self.assertIn('minute', t)
        self.assertNotIn('hour', t)
        self.assertEqual(t.attributes, dict(minute=24))
        t['minute'] = 36
        self.assertEqual(t.attributes, dict(minute=36))
        t['second'] = 92
        self.assertEqual(t.attributes, dict(minute=36, second=92))

    def testUnderscore(self):
        class Node(base.NodeBase):
            PROPERTIES = [base.Property('class_')]
        n = Node(class_="foo")
        self.assertEqual(n.class_, "foo")

    def testPropertiesError(self):
        class NotList(base.NodeBase):
            PROPERTIES = 'not list'
        with self.assertRaises(exceptions.MooseDocsException) as e:
            NotList()
        gold = "The class attribute 'PROPERTIES' must be a list."
        self.assertEqual(e.exception.message, gold)

        class NotProperty(base.NodeBase):
            PROPERTIES = ['not prop']
        with self.assertRaises(exceptions.MooseDocsException) as e:
            NotProperty()
        gold = "The supplied property must be a Property object, but str provided."
        self.assertEqual(e.exception.message, gold)

    @unittest.skip("Not ready, may not be possible")
    def testPropExistsError(self):
        class TestNode(base.NodeBase):
            PROPERTIES = [base.Property('root')]

        with self.assertRaises(exceptions.MooseDocsException) as e:
            TestNode(None)
        gold = "The supplied property 'root' is already a defined property on the TestNode object."
        self.assertEqual(e.exception.message, gold)

if __name__ == '__main__':
    unittest.main(verbosity=2)
