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
import moosetree
import mooseutils
import moosesyntax

class TestSyntaxTreeRun(unittest.TestCase):
    """Test 'get_moose_syntax_tree' can operate with given executable"""
    def testRun(self):
        location = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'test'))
        exe = mooseutils.find_moose_executable(location)
        root = moosesyntax.get_moose_syntax_tree(exe)

class TestSyntaxTree(unittest.TestCase):
    """Test 'get_moose_syntax_tree' features"""
    @classmethod
    def setUpClass(cls):
        location = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'test'))
        exe = mooseutils.find_moose_executable(location)
        raw = mooseutils.runExe(exe, ['--json', '--allow-test-objects'])
        raw = raw.split('**START JSON DATA**\n')[1]
        raw = raw.split('**END JSON DATA**')[0]
        cls.json = mooseutils.json_parse(raw)

    def testNodes(self):
        root = moosesyntax.get_moose_syntax_tree(self.json)

        # SyntaxNode
        node = moosetree.find(root, lambda n: n.fullpath() == '/Adaptivity')
        self.assertEqual(node.name, 'Adaptivity')
        self.assertIsInstance(node, moosesyntax.SyntaxNode)
        self.assertEqual(node.hidden, False)
        self.assertEqual(node.removed, False)
        self.assertEqual(node.group, None)
        self.assertEqual(node.groups(), {'MooseApp'})
        self.assertIn('SetAdaptivityOptionsAction', [action.name for action in node.actions()])
        self.assertIn('Indicators', [syntax.name for syntax in node.syntax()])

        node = moosetree.find(root, lambda n: n.fullpath() == '/Adaptivity/Markers')
        self.assertIn('BoxMarker', [obj.name for obj in node.objects()])

        # MooseObjectNode
        node = moosetree.find(root, lambda n: n.fullpath() == '/Functions/ParsedFunction')
        self.assertEqual(node.name, 'ParsedFunction')
        self.assertIsInstance(node, moosesyntax.MooseObjectNode)
        self.assertEqual(node.hidden, False)
        self.assertEqual(node.removed, False)
        self.assertEqual(node.alias, None)
        self.assertEqual(node.group, 'MooseApp')
        self.assertEqual(node.classname, 'MooseParsedFunction')
        self.assertIn('Function', node.description)
        self.assertTrue(node.source.endswith('framework/src/functions/MooseParsedFunction.C'))
        self.assertTrue(node.header.endswith('framework/include/functions/MooseParsedFunction.h'))
        self.assertIsInstance(node.parameters, dict)
        self.assertIn('value', node.parameters)

        # ActionNode
        node = moosetree.find(root, lambda n: n.fullpath() == '/Outputs/CommonOutputAction')
        self.assertEqual(node.name, 'CommonOutputAction')
        self.assertIsInstance(node, moosesyntax.ActionNode)
        self.assertEqual(node.hidden, False)
        self.assertEqual(node.removed, False)
        self.assertEqual(node.group, 'MooseApp')
        self.assertIn('common_output', node.tasks)

        # MooseObjectActionNode
        node = moosetree.find(root, lambda n: n.fullpath() == '/Outputs/AddOutputAction')
        self.assertEqual(node.name, 'AddOutputAction')
        self.assertIsInstance(node, moosesyntax.ActionNode)
        self.assertEqual(node.hidden, False)
        self.assertEqual(node.removed, False)
        self.assertEqual(node.group, 'MooseApp')
        self.assertIn('add_output', node.tasks)

    def testRemove(self):

        # Test with flat list
        root = moosesyntax.get_moose_syntax_tree(self.json, remove=['/Outputs', '/Kernels/Diffusion'])
        node = moosetree.find(root, lambda n: n.fullpath() == '/Outputs')
        self.assertTrue(all([n.removed for n in node])) # children should be marked

        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels')
        self.assertFalse(node.removed)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels/Diffusion')
        self.assertTrue(node.removed)

        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels/ADDiffusion')
        self.assertFalse(node.removed)

        # Test with dict of lists
        remove = dict(first=['/Outputs', '/Kernels/Diffusion'], second=['/Kernels/ADDiffusion'])
        root = moosesyntax.get_moose_syntax_tree(self.json, remove=remove)

        node = moosetree.find(root, lambda n: n.fullpath() == '/Outputs')
        self.assertTrue(all([n.removed for n in node])) # children should be marked

        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels')
        self.assertFalse(node.removed)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels/Diffusion')
        self.assertTrue(node.removed)

        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels/ADDiffusion')
        self.assertTrue(node.removed)

    def testAlias(self):
        root = moosesyntax.get_moose_syntax_tree(self.json, alias={'/Kernels/Diffusion':'/Physics/Diffusion'})
        node = moosetree.find(root, lambda n: n.fullpath() == '/Kernels/Diffusion')
        self.assertIn(node.alias, '/Physics/Diffusion')

    def testUnregister(self):
        # Without unregister
        root = moosesyntax.get_moose_syntax_tree(self.json)
        node = moosetree.find(root, lambda n: n.fullpath() == '/UserObjects/AreaPostprocessor')
        self.assertEqual(node['moose_base'], 'Postprocessor')
        self.assertEqual(node['parent_syntax'], 'UserObjects/*')
        self.assertFalse(node.removed)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Bounds/ConstantAux')
        self.assertFalse(node.removed)

        # With unregister(objects should be removed)
        root = moosesyntax.get_moose_syntax_tree(self.json, unregister={'Postprocessor':'UserObjects/*', 'AuxKernel':'Bounds/*'})
        node = moosetree.find(root, lambda n: n.fullpath() == '/UserObjects/AreaPostprocessor')
        self.assertTrue(node.removed)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Bounds/ConstantAux')
        self.assertTrue(node.removed)

        # With dict of dict unregister
        unregister={'framework':{'Postprocessor':'UserObjects/*', 'AuxKernel':'Bounds/*'}}
        root = moosesyntax.get_moose_syntax_tree(self.json, unregister=unregister)
        node = moosetree.find(root, lambda n: n.fullpath() == '/UserObjects/AreaPostprocessor')
        self.assertTrue(node.removed)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Bounds/ConstantAux')
        self.assertTrue(node.removed)

    def testTestApp(self):
        root = moosesyntax.get_moose_syntax_tree(self.json)
        node = moosetree.find(root, lambda n: n.fullpath() == '/Testing')
        self.assertTrue(node.test)
        self.assertTrue(node(0).test)
        self.assertTrue(node(0,0).test)

    def testMarkdown(self):
        root = moosesyntax.get_moose_syntax_tree(self.json, markdown={'/AuxKernels/ADMaterialRealAux' : 'auxkernels/ADMaterialRealAux.md'})
        node = moosetree.find(root, lambda n: n.fullpath() == '/AuxKernels/ADMaterialRealAux')
        self.assertEqual(node.markdown, 'auxkernels/ADMaterialRealAux.md')

if __name__ == '__main__':
    unittest.main(verbosity=2)
