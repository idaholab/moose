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
import mock
import types
import io
import mooseutils
import moosesqa
import copy
import MooseDocs
from MooseDocs.commands import build
from MooseDocs import base, common

class TestBuild(unittest.TestCase):
    def setUp(self):
        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

    def getCommandLineArguments(self, **kwargs):
        """Mimic build command_line_options"""
        kwargs.setdefault('config', 'config.yml')
        kwargs.setdefault('args', None)
        kwargs.setdefault('disable', [])
        kwargs.setdefault('fast', False)
        kwargs.setdefault('executioner', 'MooseDocs.base.ParallelQueue')
        kwargs.setdefault('profile', False)
        kwargs.setdefault('destination', None)
        kwargs.setdefault('serve', False)
        kwargs.setdefault('dump', False)
        kwargs.setdefault('num_threads', 1)
        kwargs.setdefault('port', '8000')
        kwargs.setdefault('host', '127.0.0.1')
        kwargs.setdefault('clean', '0')
        kwargs.setdefault('files', [])
        kwargs.setdefault('home', None)
        kwargs.setdefault('stable', False)
        kwargs.setdefault('hide_source', False)
        return types.SimpleNamespace(**kwargs)

    # Note: mock.patch.object() decorators are applied from the bottom upwards
    @mock.patch.object(base.Translator, 'execute')
    @mock.patch.object(base.Translator, 'init')
    def testBuildDefault(self, init_mock, execute_mock):
        opt = self.getCommandLineArguments()
        status = build.main(opt)
        init_mock.assert_called_once()
        execute_mock.assert_called()

    @mock.patch.object(base.Translator, 'execute')
    @mock.patch.object(base.Translator, 'init')
    def testBuildConfigEdit(self, init_mock, execute_mock):
        load_configs = common.load_configs # get a static refernce to the real load_configs()

        # Unmodified
        def load_unmodified(filenames, **kwargs):
            trans, _, confs = load_configs(filenames, **kwargs)
            self.assertNotIn('MooseDocs.extensions.listing', confs[0]['Extensions'])
            for ext in trans[0].extensions:
                if isinstance(ext, MooseDocs.extensions.listing.ListingExtension):
                    self.assertTrue(ext['modal-link'])
            return trans, [[]], confs

        opt = self.getCommandLineArguments()
        with mock.patch('MooseDocs.common.load_configs', load_unmodified):
            status = build.main(opt)

        # Modified
        def load_modified(filenames, **kwargs):
            trans, _, confs = load_configs(filenames, **kwargs)
            self.assertFalse(confs[0]['Extensions']['MooseDocs.extensions.listing']['modal-link'])
            for ext in trans[0].extensions:
                if isinstance(ext, MooseDocs.extensions.listing.ListingExtension):
                    self.assertFalse(ext['modal-link'])
            return trans, [[]], confs

        args = {'Extensions': {'MooseDocs.extensions.listing': {'modal-link': 0}}}
        opt = self.getCommandLineArguments(args=args)
        with mock.patch('MooseDocs.common.load_configs', load_modified):
            status = build.main(opt)

        # Multiple
        def load_multiple(filenames, **kwargs):
            trans, conts, confs = load_configs(filenames, **kwargs)

            # Assert translators
            self.assertEqual(len(trans), 2)
            self.assertIsInstance(trans[0], base.Translator)
            self.assertIsInstance(trans[1], base.Translator)
            self.assertIsNot(*trans)
            self.assertEqual(trans[0].destination, trans[1].destination)

            # Assert that translators have access to the same global pool of non-directory content
            isdir = lambda p: isinstance(p, MooseDocs.tree.pages.Directory)
            sort_local = lambda p: p.local
            page_list = [sorted([p for p in trans[0].getPages() if not isdir(p)], key=sort_local)]
            page_list += [sorted([p for p in trans[1].getPages() if not isdir(p)], key=sort_local)]
            self.assertListEqual(*page_list)

            # Assert build content lists
            self.assertEqual(len(conts), 2)
            self.assertIsInstance(conts[0], list)
            self.assertIsInstance(conts[1], list)
            for page in conts[0]:
                self.assertNotIn(page, conts[1])

            # Assert configurations
            self.assertEqual(len(confs), 2)
            self.assertIsInstance(confs[0], dict)
            self.assertIsInstance(confs[1], dict)
            self.assertNotEqual(*confs)

            self._load_multiple_out = (trans, conts, confs)
            return self._load_multiple_out

        testpath = '../../python/MooseDocs/test/'
        configs = [testpath + config for config in ['config.yml', 'subsite_config.yml']]
        opt = self.getCommandLineArguments(config=configs)
        with mock.patch('MooseDocs.common.load_configs', load_multiple):
            status = build.main(opt)
            _, conts, _ = self._load_multiple_out
            init_mock.assert_any_call(conts[0])
            init_mock.assert_any_call(conts[1])

if __name__ == '__main__':
    unittest.main(verbosity=2)
