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
import logging
import MooseDocs
from MooseDocs.tree import pages
from MooseDocs.common import exceptions
from MooseDocs.common.load_config import load_config, _yaml_load_extensions, _yaml_load_object, DEFAULT_EXTENSIONS

logging.basicConfig()

class TestLoadConfig(unittest.TestCase):
    def setUp(self):
        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

    def test(self):
        translator, config = load_config('config.yml')
        self.assertIn('Extensions', config)
        self.assertIn('MooseDocs.extensions.common', config['Extensions'])
        self.assertNotIn('MooseDocs.extensions.listing', config['Extensions'])

        kwargs = {'Extensions': {'MooseDocs.extensions.listing': {'modal-link': '0'}}}
        translator, config = load_config('config.yml', **kwargs)
        self.assertIn('Extensions', config)
        self.assertIn('MooseDocs.extensions.common', config['Extensions'])
        self.assertIn('MooseDocs.extensions.listing', config['Extensions'])
        self.assertIn('modal-link', config['Extensions']['MooseDocs.extensions.listing'])
        self.assertEqual(config['Extensions']['MooseDocs.extensions.listing']['modal-link'], '0')

class TestLoadExtensions(unittest.TestCase):
    def testEmpty(self):
        ext = _yaml_load_extensions(dict())
        names = set([e.__module__ for e in ext])
        self.assertEqual(names, set(DEFAULT_EXTENSIONS))

    def testDisableDefaults(self):
        config = dict(Extensions=dict(disable_defaults=True))
        ext = _yaml_load_extensions(config)
        self.assertEqual(ext, [])

    def testModuleImportError(self):
        config = dict(Extensions=dict(disable_defaults=True, foo='default'))
        with self.assertRaises(exceptions.MooseDocsException) as e:
            _yaml_load_extensions(config)

        self.assertIn("Failed to import the supplied 'foo' module.", str(e.exception))

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
        content = [pages.Page('foo', source='foo')]
        reader = MooseDocs.base.MarkdownReader()
        renderer = MooseDocs.base.HTMLRenderer()
        obj = _yaml_load_object('Translator', dict(), 'MooseDocs.base.Translator', content, reader, renderer, [])
        self.assertIsInstance(obj, MooseDocs.base.Translator)

    def testNode(self):
        content = [pages.Page('foo', source='foo')]
        reader = MooseDocs.base.MarkdownReader()
        renderer = MooseDocs.base.HTMLRenderer()
        config = dict(Translator=dict(type='MooseDocs.base.Translator'))
        obj = _yaml_load_object('Translator', config, None, content, reader, renderer, [])
        self.assertIsInstance(obj, MooseDocs.base.Translator)


if __name__ == '__main__':
    unittest.main(verbosity=2)
