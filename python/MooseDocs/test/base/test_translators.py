#!/usr/bin/env python2
"""
Testing for Translator object.
"""
import unittest
from MooseDocs.tree import page
from MooseDocs.base import Translator, MarkdownReader, HTMLRenderer
from MooseDocs.common import exceptions

class TestTranslator(unittest.TestCase):
    """
    Test basic functionality and error handling of Translator object.
    """
    def testConstruction(self):
        """
        Test most basic construction.
        """
        content = page.PageNodeBase(None)
        translator = Translator(content, MarkdownReader(), HTMLRenderer(), [])
        self.assertIsInstance(translator.reader, MarkdownReader)
        self.assertIsInstance(translator.renderer, HTMLRenderer)

    def testConstructionTypeError(self):
        """
        Test type error for reader/renderer arguments.
        """

        # Content
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Translator(None, 'foo', HTMLRenderer(), [])
        self.assertIn("The argument 'content' must be", e.exception.message)

        # Reader
        content = page.PageNodeBase(None)
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Translator(content, 'foo', HTMLRenderer(), [])
        self.assertIn("The argument 'reader' must be", e.exception.message)

        # Renderer
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Translator(content, MarkdownReader(), 'foo', [])
        self.assertIn("The argument 'renderer' must be", e.exception.message)

        # Extensions
        with self.assertRaises(exceptions.MooseDocsException) as e:
            Translator(content, MarkdownReader(), HTMLRenderer(), ['foo'])
        self.assertIn("The argument 'extensions' must be", e.exception.message)

if __name__ == '__main__':
    unittest.main(verbosity=2)
