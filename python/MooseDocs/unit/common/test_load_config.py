#!/usr/bin/env python2
import unittest
import mock

import MooseDocs
from MooseDocs.tree import page
from MooseDocs.common import exceptions
from MooseDocs.common.load_config import _yaml_load_extensions, _yaml_load_object, DEFAULT_EXTENSIONS


class TestLoadExtensions(unittest.TestCase):
    def testEmpty(self):
        ext = _yaml_load_extensions(dict())
        names = set([e.__module__ for e in ext])
        self.assertEqual(names, set(DEFAULT_EXTENSIONS))

    def testDisableDefaults(self):
        config = dict(Extensions=dict(disable_defaults=True))
        ext = _yaml_load_extensions(config)
        self.assertEqual(ext, [])

    @mock.patch('logging.Logger.error')
    def testMissingTypeError(self, mock):
        config = dict(Extensions=dict(disable_defaults=True, foo=dict()))
        ext = _yaml_load_extensions(config)
        self.assertEqual(ext, [])

        mock.assert_called_once()
        args, _ = mock.call_args
        self.assertIn("The section '%s' must contain a 'type' parameter.", args[0])

    def testModuleImportError(self):
        config = dict(Extensions=dict(disable_defaults=True, foo=dict(type='Bad')))
        with self.assertRaises(exceptions.MooseDocsException) as e:
            _yaml_load_extensions(config)

        self.assertIn("Failed to import the supplied 'Bad' module.", e.exception.message)

class TestLoadReader(unittest.TestCase):
    def testEmpty(self):
        obj = _yaml_load_object('Reader', dict(), 'MooseDocs.base.MarkdownReader')
        self.assertIsInstance(obj, MooseDocs.base.MarkdownReader)

    def testNode(self):
        config = dict(Reader=dict(type='MooseDocs.base.MarkdownReader'))
        obj = _yaml_load_object('Reader', config, None)
        self.assertIsInstance(obj, MooseDocs.base.MarkdownReader)

class TestLoadRenderer(unittest.TestCase):
    def testEmpty(self):
        obj = _yaml_load_object('Renderer', dict(), 'MooseDocs.base.MaterializeRenderer')
        self.assertIsInstance(obj, MooseDocs.base.MaterializeRenderer)

    def testNode(self):
        config = dict(Renderer=dict(type='MooseDocs.base.MaterializeRenderer'))
        obj = _yaml_load_object('Renderer', config, None)
        self.assertIsInstance(obj, MooseDocs.base.MaterializeRenderer)

class TestLoadTranslator(unittest.TestCase):
    def testEmpty(self):
        content = page.PageNodeBase(None)
        reader = MooseDocs.base.MarkdownReader()
        renderer = MooseDocs.base.HTMLRenderer()
        obj = _yaml_load_object('Translator', dict(), 'MooseDocs.base.Translator', content, reader, renderer, [])
        self.assertIsInstance(obj, MooseDocs.base.Translator)

    def testNode(self):
        content = page.PageNodeBase(None)
        reader = MooseDocs.base.MarkdownReader()
        renderer = MooseDocs.base.HTMLRenderer()
        config = dict(Translator=dict(type='MooseDocs.base.Translator'))
        obj = _yaml_load_object('Translator', config, None, content, reader, renderer, [])
        self.assertIsInstance(obj, MooseDocs.base.Translator)

if __name__ == '__main__':
    unittest.main(verbosity=2)
