#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tests for Component objects.
"""
import unittest
import mock

from MooseDocs.tree import tokens, pages
from MooseDocs.common import exceptions
from MooseDocs.base.components import ReaderComponent
from MooseDocs.base.lexers import RecursiveLexer, LexerInformation
from MooseDocs.base import Translator, Reader, Renderer, Extension

class TestExtension(unittest.TestCase):
    """
    Test the Extension class.
    """
    def testExtend(self):
        """
        Test the extend method.
        """
        class ExtTester(Extension):
            """Dummy extension for testing."""
            def __init__(self, *args, **kwargs):
                Extension.__init__(self, *args, **kwargs)
                self.called = False
            def extend(self, reader, renderer):
                self.called = True

        ext = ExtTester()
        self.assertFalse(ext.called)
        content = pages.Page('foo', source='foo')
        t = Translator([content], Reader(RecursiveLexer('foo')), Renderer(), [ext])
        t.init()
        self.assertTrue(ext.called)

class TestReaderComponent(unittest.TestCase):
    """
    Test ReaderComponent.
    """
    def testDefault(self):
        """
        Test basic construction and errors.
        """
        comp = ReaderComponent()
        defaults = comp.defaultSettings()
        for key in ['id', 'class', 'style']:
            self.assertIn(key, defaults)
            self.assertIsInstance(defaults[key], tuple)
            self.assertEqual(len(defaults[key]), 2)
            self.assertEqual(defaults[key][0], None)
            self.assertIsInstance(defaults[key][1], str)

    def testExceptions(self):
        """
        Test that exceptions are raised.
        """
        comp = ReaderComponent()

        with self.assertRaises(NotImplementedError):
            comp.createToken(None, None, None, None)

        # Test defaultSettings return type check
        class TestToken(ReaderComponent):
            @staticmethod
            def defaultSettings():
                pass

        with self.assertRaises(exceptions.MooseDocsException) as e:
            TestToken()
        self.assertIn("must return a dict", str(e.exception))

    def testCreateToken(self):
        """
        Test the createToken method is called.
        """
        class TestToken(ReaderComponent):
            PARSE_SETTINGS = False
            def createToken(self, *args):
                self.count = 1

        info = mock.Mock(spec=LexerInformation)
        parent = tokens.Token()
        comp = TestToken()
        comp(parent, info, None)
        self.assertEqual(comp.count, 1)

if __name__ == '__main__':
    unittest.main(verbosity=2)
