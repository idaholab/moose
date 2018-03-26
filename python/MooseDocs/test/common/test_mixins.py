#!/usr/bin/env python2
#pylint: disable=missing-docstring

import unittest
from MooseDocs.tree import page
from MooseDocs.common import exceptions, mixins
from MooseDocs.base import Translator, MarkdownReader, HTMLRenderer

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
    def testConstruction(self):
        """
        Test most basic construction.
        """
        obj = mixins.ConfigObject()
        self.assertEqual(obj.getConfig(), dict())

    def testDefaultConfig(self):
        """
        Test defaultConfig returns class level options.
        """
        obj = Foo()
        self.assertEqual(obj.getConfig(), dict(foo='bar'))

    def testBadDefaultConfigReturn(self):
        """
        Test exception from defaultConfig.
        """
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Bar()
        self.assertIn("The return type from 'defaultConfig'", e.exception.message)

    def testUpdateAndGet(self):
        """
        Test update method.
        """
        obj = Foo()
        obj.update(foo='foo')
        self.assertEqual(obj.getConfig(), dict(foo='foo'))
        self.assertEqual(obj['foo'], 'foo')
        self.assertEqual(obj.get('foo'), 'foo')
        self.assertIsNone(obj.get('bar', None))

    def testUnknown(self):
        """
        Test unknown config exception.
        """
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Foo(unknown=42)
        self.assertIn("The following config options", e.exception.message)
        self.assertIn("unknown", e.exception.message)

class TestTranslatorObject(unittest.TestCase):
    """
    Test basic use of TranslatorObject.
    """
    def testBasic(self):
        """
        Test correct use.
        """
        t = Translator(page.PageNodeBase(None), MarkdownReader(), HTMLRenderer(), [])
        obj = mixins.TranslatorObject()
        self.assertFalse(obj.initialized())
        obj.init(t)
        self.assertTrue(obj.initialized())
        self.assertIs(obj.translator, t)

    def testExceptions(self):
        """
        Test Exceptions.
        """
        t = Translator(page.PageNodeBase(None), MarkdownReader(), HTMLRenderer(), [])
        obj = mixins.TranslatorObject()
        with self.assertRaises(exceptions.MooseDocsException) as e:
            obj.init('')
        self.assertIn("The argument 'translator'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            obj.translator
        self.assertIn("The init() method of", e.exception.message)

        obj.init(t)
        with self.assertRaises(exceptions.MooseDocsException) as e:
            obj.init(t)
        self.assertIn("already been initialized", e.exception.message)

if __name__ == '__main__':
    unittest.main(verbosity=2)
