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
from MooseDocs import base

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
        kwargs.setdefault('executioner', 'MooseDocs.base.ParallelBarrier')
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
        return types.SimpleNamespace(**kwargs)

    @mock.patch.object(base.Translator, 'execute')
    @mock.patch.object(base.Translator, 'update')
    @mock.patch.object(base.Translator, 'init')
    def testBuildDefault(self, execute_mock, update_mock, init_mock):
        opt = self.getCommandLineArguments()
        status = build.main(opt)
        update_mock.assert_called_once()
        init_mock.assert_called_once()
        execute_mock.assert_called_once()

    @mock.patch.object(base.Translator, 'execute')
    @mock.patch.object(base.Translator, 'update')
    @mock.patch.object(base.Translator, 'init')
    def testBuildConfigEdit(self, execute_mock, update_mock, init_mock):
        load_func = MooseDocs.common.load_config

        # Un-modified
        opt = self.getCommandLineArguments()
        def load_un_modifed(*args, **kwargs):
            t, c = load_func(*args, **kwargs)
            self.assertNotIn('MooseDocs.extensions.listing', c['Extensions'])
            for ext in t.extensions:
                if isinstance(ext, MooseDocs.extensions.listing.ListingExtension):
                    self.assertTrue(ext['modal-link'])
            return t, c

        with mock.patch('MooseDocs.common.load_config', wraps=load_un_modifed):
            status = build.main(opt)

        # Modified
        opt = self.getCommandLineArguments(args={'Extensions': {'MooseDocs.extensions.listing': {'modal-link': 0}}})
        def load_modified(*args, **kwargs):
            t, c = load_func(*args, **kwargs)
            self.assertFalse(c['Extensions']['MooseDocs.extensions.listing']['modal-link'])
            for ext in t.extensions:
                if isinstance(ext, MooseDocs.extensions.listing.ListingExtension):
                    self.assertFalse(ext['modal-link'])
            return t, c

        with mock.patch('MooseDocs.common.load_config', wraps=load_modified):
            status = build.main(opt)

if __name__ == '__main__':
    unittest.main(verbosity=2)
