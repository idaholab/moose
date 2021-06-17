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
import re
import unittest
import mock
import types
import MooseDocs
from MooseDocs.commands import init

class TestInit(unittest.TestCase):
    def setUp(self):
        self._init_write = dict()
        self._yaml_write = dict()

        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

        self._init_write = None
        self._yanl_write = None

    def getCommandLineArguments(self, **kwargs):

        kwargs.setdefault('config', os.path.join(os.getcwd(), 'config.yml'))
        kwargs.setdefault('command', 'init')
        kwargs.setdefault('init_command', 'sqa')

        return types.SimpleNamespace(**kwargs)

    def mockInitWrite(self, filename, content):
        self._init_write[filename.replace(MooseDocs.ROOT_DIR, '')] = content

    def mockYamlWrite(self, filename, content):
        self._yaml_write[filename.replace(MooseDocs.ROOT_DIR, '')] = content

    @mock.patch('mooseutils.git_repo')
    @mock.patch('mooseutils.IncludeYamlFile')
    @mock.patch('mooseutils.yaml_write')
    @mock.patch('MooseDocs.commands.init._write_file')
    def testSQA(self, mock_write_file, mock_yaml_write,proxy, mock_git_repo):
        mock_yaml_write.side_effect = self.mockYamlWrite
        mock_write_file.side_effect = self.mockInitWrite
        mock_git_repo.return_value = 'test'

        opt = self.getCommandLineArguments(app='MooseTestApp', category='moose_test')
        status = init.main(opt)

        self.assertEqual(len(self._init_write.keys()), 12)

        self.assertIn('/test/doc/sqa_moose_test.yml', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_sdd.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_srs.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_stp.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/index.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_rtm.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_vvr.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_far.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_scs.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_cci.md', self._init_write.keys())
        self.assertIn('/test/doc/content/sqa/moose_test_sll.md', self._init_write.keys())
        self.assertIn('/test/doc/sqa_reports.yml', self._init_write.keys())

        self.assertEqual(len(self._yaml_write.keys()), 1)
        self.assertIn('/test/doc/config.yml', self._yaml_write.keys())

if __name__ == '__main__':
    unittest.main(verbosity=2)
