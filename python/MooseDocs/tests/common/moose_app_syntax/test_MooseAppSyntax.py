#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import os
import unittest

import MooseDocs
from MooseDocs.common import moose_docs_app_syntax
from MooseDocs.common.nodes import SyntaxNode, MooseObjectNode, ActionNode, MooseObjectActionNode

class TestMooseAppSyntax(unittest.TestCase):
    """
    TestCase for MooseAppSyntax class.
    """

    @classmethod
    def setUpClass(cls):
        exe = os.path.join(MooseDocs.ROOT_DIR, 'modules', 'combined')
        hide = {'framework': ['/Functions', '/Functions/ParsedFunction',
                              '/Functions/AddFunctionAction'],
                'all': ['/Modules/PhaseField', '/Modules/PhaseField/EulerAngles2RGB']}
        cls._syntax = moose_docs_app_syntax(exe, hide)

    def testFindall(self):
        """
        Test findall method.
        """
        nodes = self._syntax.findall('Diffusion')
        self.assertIsInstance(nodes, list)

        for syntax in [u'/Kernels/MatDiffusion', u'/Kernels/Diffusion', u'/Kernels/Diffusion']:
            found = False
            for n in nodes:
                if n.full_name == syntax:
                    found = True
                    break
            self.assertTrue(found, 'Failed to locate: {}'.format(syntax))

        return nodes

    def testFind(self):
        """
        Test finding various node types.
        """

        # MooseObject
        obj = self._syntax.findall('/Kernels/Diffusion')[0]
        self.assertIsInstance(obj, MooseObjectNode)
        self.assertEqual(obj.full_name, u'/Kernels/Diffusion')

        # MooseObjectAction
        moa = self._syntax.findall('/Kernels/AddKernelAction')[0]
        self.assertIsInstance(moa, MooseObjectActionNode)
        self.assertEqual(moa.full_name, u'/Kernels/AddKernelAction')
        self.assertIsInstance(moa.parameters, dict)
        self.assertIn('isObjectAction', moa.parameters)

        # Action
        act = self._syntax.findall('/Outputs/CommonOutputAction')[0]
        self.assertIsInstance(act, ActionNode)
        self.assertEqual(act.full_name, u'/Outputs/CommonOutputAction')
        self.assertIsInstance(act.parameters, dict)
        self.assertNotIn('isObjectAction', act.parameters)

        # Syntax
        syntax = self._syntax.findall('/Kernels')[0]
        self.assertEqual(syntax.full_name, u'/Kernels')

    def testParameters(self):
        """
        Test parameters access.
        """
        node = self._syntax.findall('/Kernels/Diffusion')[0]
        self.assertIsInstance(node.parameters, dict)
        self.assertEqual(node.parameters['type']['default'], 'Diffusion')

    def testDescription(self):
        """
        Test description access.
        """
        node = self._syntax.findall('/Kernels/Diffusion')[0]
        self.assertIsInstance(node.description, unicode)
        self.assertIn('Laplacian', node.description)

    def testObjects(self):
        """
        Test that MooseObjects can be found for given syntax.
        """
        nodes = self._syntax.objects('/Kernels', recursive=True)
        self.assertTrue(len(nodes))
        for node in nodes:
            self.assertNotIsInstance(node, (ActionNode, MooseObjectActionNode))

    def testActions(self):
        """
        Test that Actions can be located for given syntax.
        """
        nodes = self._syntax.actions('/Kernels', recursive=True)
        self.assertTrue(len(nodes))
        for node in nodes:
            self.assertNotIsInstance(node, MooseObjectNode)

    def testNotFound(self):
        """
        Test that findall exits when bad syntax is given.
        """
        nodes = self._syntax.findall('/NOT/VALID/SYNTAX')
        self.assertEqual(len(nodes), 0)

    def testNamedObjects(self):
        """
        Test that named objects are handled.
        """
        nodes = self._syntax.findall('/Functions/ParsedFunction')
        self.assertTrue(len(nodes) == 1)
        self.assertIsInstance(nodes[0], MooseObjectNode)
        self.assertEqual(nodes[0].name, 'ParsedFunction')
        self.assertEqual(nodes[0].class_name, 'MooseParsedFunction')

    def testAppName(self):
        """
        Test that the app name is discovered.
        """
        node = self._syntax.findall('/Kernels/Diffusion')[0]
        self.assertEqual(node.groups, {u'framework':u'Framework'})

        node = self._syntax.findall('/Kernels/LevelSetAdvection')[0]
        self.assertEqual(node.groups, {'level_set':'Level Set'})

        node = self._syntax.findall('/Outputs/CommonOutputAction')[0]
        self.assertEqual(node.groups, {'moose':'Moose', 'framework':'Framework'})

        node = self._syntax.findall('/Kernels/AddKernelAction')[0]
        self.assertEqual(node.groups, {u'framework':u'Framework'})

    def testGroups(self):
        """
        Test that the group method is working.
        """
        node = self._syntax.findall('/Functions')[0]
        self.assertIsInstance(node, SyntaxNode)
        self.assertEqual(node.groups, {'moose':'Moose', 'framework':'Framework'})

        node = self._syntax.findall('/Functions/LevelSetOlssonBubble')[0]
        self.assertIsInstance(node, MooseObjectNode)
        self.assertEqual(node.groups, {'level_set':'Level Set'})

        node = self._syntax.findall('/Functions/ImageFunction')[0]
        self.assertIsInstance(node, MooseObjectNode)
        self.assertEqual(node.groups, {'framework':'Framework'})

    def testHidden(self):
        """
        Test the actions, syntax, and objects can be hidden.
        """
        node = self._syntax.findall('/Functions')[0]
        self.assertTrue(node.hidden)

        node = self._syntax.findall('/Functions/ParsedFunction')[0]
        self.assertTrue(node.hidden)

        node = self._syntax.findall('/Functions/AddFunctionAction')[0]
        self.assertTrue(node.hidden)

        node = self._syntax.findall('/Adaptivity/Markers')[0]
        self.assertFalse(node.hidden)

        node = self._syntax.findall('/Adaptivity/Markers/BoxMarker')[0]
        self.assertFalse(node.hidden)

        node = self._syntax.findall('/Adaptivity/Markers/AddMarkerAction')[0]
        self.assertFalse(node.hidden)

        node = self._syntax.findall('/Adaptivity/Markers/AddMarkerAction')[0]
        self.assertFalse(node.hidden)

        node = self._syntax.findall('/Modules/PhaseField')[0]
        self.assertTrue(node.hidden)

        node = self._syntax.findall('/Modules/PhaseField/EulerAngles2RGB')[0]
        self.assertTrue(node.hidden)

    def testPostprocessorAndUserObjects(self):
        """
        Test that Postprocessors don't show up as UserObjects.
        """
        nodes = self._syntax.findall('UserObjects/NumVars')
        self.assertNotEqual(nodes, [])
        self.assertTrue(nodes[0].hidden)

    def testActionGroups(self):
        """
        Test that groups are assigned to Actions.
        """
        nodes = self._syntax.findall('/AddMarkerAction')
        self.assertTrue(nodes[0].groups, ['framework'])

if __name__ == '__main__':
    unittest.main(verbosity=2)
