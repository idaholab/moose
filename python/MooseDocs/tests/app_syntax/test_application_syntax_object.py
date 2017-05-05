#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################

import os
import unittest

import mooseutils

import MooseDocs
from MooseDocs.common.MooseApplicationSyntax import MooseApplicationSyntax, MooseObjectInfo

class TestMooseApplicationSyntax(unittest.TestCase):

    @classmethod
    def setUpClass(cls):

        # Read the configuration
        config = MooseDocs.load_config(os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'website.yml'))
        options = config['MooseDocs.extensions.app_syntax']

        # Extract the MOOSE YAML data
        exe = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined', 'combined-opt')
        raw = mooseutils.runExe(exe, '--yaml')
        cls._yaml = mooseutils.MooseYaml(raw)

        # Extract the 'framework' location options and build the syntax object
        framework = options['locations'][0]['framework']
        framework['group'] = 'framework'
        framework['install'] = options['install']
        framework['hide'] = ['/AuxKernels'] # use to test hide
        cls._syntax = MooseApplicationSyntax(cls._yaml, **framework)

    def testHasObjects(self):
        self.assertTrue(self._syntax.hasObject('/Kernels/Diffusion'))
        self.assertFalse(self._syntax.hasObject('NotAnObjectThatExistsInMoose'))

    def testGetObjects(self):
        obj = self._syntax.getObject('/Kernels/Diffusion')
        self.assertIsInstance(obj, MooseObjectInfo)

        with self.assertRaises(KeyError):
            obj = self._syntax.getObject('NotAnObjectThatExistsInMoose')

    def testMooseObjectInfo(self):
        obj = self._syntax.getObject('/Adaptivity/Markers/BoxMarker')
        h = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'include', 'markers', 'BoxMarker.h')
        c = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'src', 'markers', 'BoxMarker.C')
        md = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems',
                          'Adaptivity', 'Markers', 'framework', 'BoxMarker.md')

        self.assertEqual(obj.name, 'BoxMarker')
        self.assertEqual(obj.syntax, '/Adaptivity/Markers/BoxMarker')
        self.assertEqual(obj.key, '/Adaptivity/Markers/BoxMarker')
        self.assertEqual(obj.code, [h, c])
        self.assertFalse(obj.hidden)
        self.assertIsInstance(obj.parameters, list)
        self.assertIn('Marks the region inside and outside', obj.description)
        self.assertEqual(obj.markdown, md)

    def testMooseObjectInfoHidden(self):
        obj = self._syntax.getObject('/AuxKernels/ConstantAux')
        self.assertTrue(obj.hidden)

    def testMooseObjectInfoType(self):
        obj = self._syntax.getObject('/Mesh/GeneratedMesh')
        self.assertEqual(obj.key, '/Mesh/GeneratedMesh')
        self.assertEqual(obj.syntax, '/Mesh/<type>/GeneratedMesh')

    def testMooseActionInfo(self):
        obj = self._syntax.getAction('/Mesh')
        md = os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'content', 'documentation', 'systems',
                          'Mesh', 'index.md')
        inc = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'include', 'actions')
        src = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'src', 'actions')
        code = []
        for name in ['SetupMeshCompleteAction', 'SetupMeshAction', 'CreateDisplacedProblemAction']:
            code.append(os.path.join(inc, name + '.h'))
            code.append(os.path.join(src, name + '.C'))

        self.assertEqual(obj.key, '/Mesh')
        self.assertEqual(obj.name, 'Mesh')
        self.assertEqual(obj.syntax, '/Mesh')
        self.assertEqual(obj.code, code)
        self.assertFalse(obj.hidden)
        self.assertIsInstance(obj.parameters, list)
        self.assertEqual(obj.markdown, md)

    def testMooseActionInfoHidden(self):
        obj = self._syntax.getAction('/AuxKernels')
        self.assertTrue(obj.hidden)

    def testObjects(self):
        objects = self._syntax.objects(prefix='/Kernels/')
        for obj in objects:
            self.assertTrue(obj.key.startswith('/Kernels/'))

    def testActions(self):
        objects = self._syntax.actions(prefix='/Adaptivity/')
        for obj in objects:
            self.assertTrue(obj.key.startswith('/Adaptivity/'))


if __name__ == '__main__':
    unittest.main(verbosity=2)
