#!/usr/bin/env python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import unittest
import mock
import logging
import mooseutils
import moosesyntax
from moosesqa import check_syntax

class TestCheckSyntax(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
        cls._cache = mooseutils.git_ls_files(cls.MOOSE_DIR)

        cls.file_is_stub = getattr(sys.modules['moosesqa.check_syntax'], 'file_is_stub')
        cls.find_md_file = getattr(sys.modules['moosesqa.check_syntax'], 'find_md_file')

    def setUp(self):
        # I was unable to get mock.patch command to work for the find_md_file and is_stub
        # functions that are in moosesqa.check_syntax.py. I think the problem is that the function
        # that needs to be patched is within the check_syntax.py file, but I load that function in
        # moosesqa.__init__, so I can't patch 'moosesqa.check_syntax.find_md_file'.
        #
        # There should be a way to use mock, but I couldn't figure it out so I am using this is
        # hack tp get the job done for a few tests.
        if self._testMethodName == 'testMissing':
            setattr(sys.modules['moosesqa.check_syntax'], 'find_md_file', lambda *args: None)
        else:
            setattr(sys.modules['moosesqa.check_syntax'], 'find_md_file', TestCheckSyntax.find_md_file)

        if self._testMethodName in ('testNonHideStub', 'testIsStub'):
            setattr(sys.modules['moosesqa.check_syntax'], 'file_is_stub', lambda *args: True)
        else:
            setattr(sys.modules['moosesqa.check_syntax'], 'file_is_stub', TestCheckSyntax.file_is_stub)


    def createSyntaxTree(self):
        root = moosesyntax.SyntaxNode(None, '')
        k = moosesyntax.SyntaxNode(root, 'Kernels', group='MooseApp')

        src = os.path.join(self.MOOSE_DIR, 'framework', 'src', 'kernels', 'Diffusion.C')
        kwargs = {'class':'Diffusion', 'description':'description', 'source':src, 'group':'MooseApp'}
        moosesyntax.MooseObjectNode(k, 'Diffusion', **kwargs)
        return root

    def testAttributes(self):
        root = self.createSyntaxTree()
        check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')

        self.assertEqual(root(0)['_md_path'], 'syntax/Kernels/index.md')
        self.assertEqual(root(0)['_md_file'], os.path.join(self.MOOSE_DIR, 'framework', 'doc', 'content', 'syntax', 'Kernels', 'index.md'))
        self.assertEqual(root(0)['_is_stub'], False)

        self.assertEqual(root(0,0)['_md_path'], 'source/kernels/Diffusion.md')
        self.assertEqual(root(0,0)['_md_file'], os.path.join(self.MOOSE_DIR, 'framework', 'doc', 'content', 'source', 'kernels', 'Diffusion.md'))
        self.assertEqual(root(0,0)['_is_stub'], False)

    def testHidden(self):

        root = self.createSyntaxTree()
        root(0,0).hidden = True

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Diffusion is marked as hidden', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_hidden=True)
        self.assertIn('Diffusion is marked as hidden', cm.output[0])

    def testIsStub(self, *args):

        root = self.createSyntaxTree()
        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('is a stub file', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_hidden=True)
        self.assertIn('is a stub file', cm.output[0])

    def testDuplicateFiles(self):

        root = self.createSyntaxTree()

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source')
        self.assertIn('Located multiple files for the desired markdown: Kernels/index.md', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', allow_duplicate_files=True)
        self.assertIn('Located multiple files for the desired markdown: Kernels/index.md', cm.output[0])

    def testHiddenAndRemoved(self):

        root = self.createSyntaxTree()
        root(0,0).hidden = True
        root(0,0).removed = True

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Diffusion is marked as both hidden and removed', cm.output[1])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_removed_and_hidden=True)
        self.assertIn('Diffusion is marked as both hidden and removed', cm.output[1])

    def testMissingDescription(self):

        root = self.createSyntaxTree()
        root(0,0).description = None

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Diffusion is missing a class description', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_missing_description=True)
        self.assertIn('Diffusion is missing a class description', cm.output[0])

    def testHideNonStub(self):

        root = self.createSyntaxTree()
        root(0,0).hidden = True

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Diffusion is hidden but the content is not a stub.', cm.output[1])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_hidden_non_stub=True)
        self.assertIn('Diffusion is hidden but the content is not a stub.', cm.output[1])

    def testMissing(self):
        root = self.createSyntaxTree()
        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('missing a markdown file', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_missing_markdown=True)
        self.assertIn('missing a markdown file', cm.output[0])

    def testNonHideStub(self):
        root = self.createSyntaxTree()
        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Kernels has a stub markdown page and is not hidden', cm.output[1])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_non_hidden_stub=True)
        self.assertIn('Kernels has a stub markdown page and is not hidden', cm.output[1])

if __name__ == '__main__':
    unittest.main(verbosity=2)
