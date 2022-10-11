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
        cls._check_node = getattr(sys.modules['moosesqa.check_syntax'], '_check_node')

    def setUp(self):
        # I was unable to get mock.patch command to work for the find_md_file and is_stub
        # functions that are in moosesqa.check_syntax.py. I think the problem is that the function
        # that needs to be patched is within the check_syntax.py file, but I load that function in
        # moosesqa.__init__, so I can't patch 'moosesqa.check_syntax.find_md_file'.
        #
        # There should be a way to use mock, but I couldn't figure it out so I am using this is
        # hack to get the job done for a few tests.
        if self._testMethodName == 'testMissing':
            setattr(sys.modules['moosesqa.check_syntax'], 'find_md_file', lambda *args: None)
        else:
            setattr(sys.modules['moosesqa.check_syntax'], 'find_md_file', TestCheckSyntax.find_md_file)

        if self._testMethodName in ('testIsStub',):
            setattr(sys.modules['moosesqa.check_syntax'], 'file_is_stub', lambda *args: True)
        else:
            setattr(sys.modules['moosesqa.check_syntax'], 'file_is_stub', TestCheckSyntax.file_is_stub)

        if self._testMethodName in ('testGiantIfStatement',):
            setattr(sys.modules['moosesqa.check_syntax'], '_check_node', self._patch_check_node)
        else:
            setattr(sys.modules['moosesqa.check_syntax'], '_check_node', TestCheckSyntax._check_node)

    @staticmethod
    def _patch_check_node(node, *args, **kwargs):
        node['called'] = True

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

    def testMissingDescription(self):

        root = self.createSyntaxTree()
        root(0,0).description = None

        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('Diffusion is missing a class description', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_missing_description=True)
        self.assertIn('Diffusion is missing a class description', cm.output[0])

    def testMissing(self):
        root = self.createSyntaxTree()
        with self.assertLogs(level='ERROR') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax')
        self.assertIn('missing a markdown file', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            check_syntax(root, ['MooseApp'], self._cache, object_prefix='source', syntax_prefix='syntax', allow_missing_markdown=True)
        self.assertIn('missing a markdown file', cm.output[0])

    def testGiantIfStatement(self):
        root = moosesyntax.SyntaxNode(None, '')
        syn = moosesyntax.SyntaxNode(root, 'Block')
        act = moosesyntax.ActionNode(syn, 'AddObject')
        obj = moosesyntax.ActionNode(syn, 'Object')

        def reset():
            for x in [syn, act, obj]:
                x['called'] = False
                x.test = False

        # Syntax/Object/Action in 'App'
        reset()
        act.group = 'App'
        obj.group = 'App'

        check_syntax(root, ['App'], [])
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertTrue(obj['called'])

        # Syntax/Action in 'App'; Object in 'Other'
        reset()
        act.group = 'App'
        obj.group = 'Other'

        check_syntax(root, ['App'], [])
        self.assertFalse(syn['called'])
        self.assertTrue(act['called'])
        self.assertFalse(obj['called'])

        # Syntax/Object/Action in 'App', Object is test
        reset()
        act.group = 'App'
        obj.group = 'App'
        obj.test = True
        check_syntax(root, ['App'], [])
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertFalse(obj['called'])

        # Syntax/Object/Action in 'App', Object is test, but tests object allowed
        check_syntax(root, ['App'], [], allow_test_objects=True)
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertTrue(obj['called'])

        # Syntax/Action in 'App'; Object in 'Other', check against 'App'
        reset()
        syn.group = 'App'
        act.group = 'App'
        obj.group = 'Other'

        check_syntax(root, ['App'], [])
        self.assertFalse(syn['called'])
        self.assertTrue(act['called'])
        self.assertFalse(obj['called'])

        # Syntax/Action in 'App'; Object in 'Other', check against 'App' and 'Other'
        check_syntax(root, ['App', 'Other'], [])
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertTrue(obj['called'])

        # Syntax/Action in 'App'; Object in 'Other', check against 'App' and 'Other', with Object as test object
        reset()
        obj.test = True
        check_syntax(root, ['App', 'Other'], [])
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertFalse(obj['called'])

        # Syntax/Action in 'App'; Object in 'Other', check against 'App' and 'Other', with Object as test object, but allowing test objects
        check_syntax(root, ['App', 'Other'], [], allow_test_objects=True)
        self.assertTrue(syn['called'])
        self.assertTrue(act['called'])
        self.assertTrue(obj['called'])

if __name__ == '__main__':
    unittest.main(verbosity=2)
