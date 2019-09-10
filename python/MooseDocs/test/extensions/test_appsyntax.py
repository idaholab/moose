#!/usr/bin/env python2
import os
import unittest
import logging

from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, table, floats, materialicon, autolink, heading, appsyntax
from MooseDocs import base
logging.basicConfig()

class AppSyntaxTestCase(MooseDocsTestCase):
    EXTENSIONS = [core, command, table, floats, materialicon, autolink, heading, appsyntax]

    def setupExtension(self, ext):
        if ext is appsyntax:
            return dict(executable=os.path.join(os.getenv('MOOSE_DIR'), 'test'))

class TestDescription(AppSyntaxTestCase):
    def testAST(self):
        ast = self.tokenize(u"!syntax description /Kernels/Diffusion")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=53)
        self.assertToken(ast(0,0), 'Word', content=u'The')
        self.assertToken(ast(0)(2), 'Word', content=u'Laplacian')

    def testError(self):
        ast = self.tokenize(u"!syntax description /Kernels/NotAKernel")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ErrorToken', size=0,
                         message=u"'/Kernels/NotAKernel' syntax was not recognized.")


class TestParameters(AppSyntaxTestCase):
    def testAST(self):
        ast = self.tokenize(u"!syntax parameters /Kernels/Diffusion")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertEqual(ast(0).text(), 'Input Parameters')
        self.assertToken(ast(1), 'InputParametersToken')

        params = ast(1)['parameters']
        self.assertIsInstance(params, dict)
        self.assertIn('enable', params)
        self.assertIn('group_name', params['enable'])
        self.assertEqual(params['enable']['group_name'], 'Advanced')

        ast = self.tokenize(u"!syntax parameters /Kernels/Diffusion heading=None")
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'InputParametersToken')

        ast = self.tokenize(u"!syntax parameters /Kernels/Diffusion heading=Foo heading-level=3")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=1, level=3)
        self.assertEqual(ast(0).text(), 'Foo')
        self.assertToken(ast(1), 'InputParametersToken')

    def testHTML(self):
        _, res = self.execute(u"!syntax parameters /Kernels/Diffusion",
                              renderer=base.HTMLRenderer())
        print res

    def testMaterialize(self):
        _, res = self.execute(u"!syntax parameters /Kernels/Diffusion",
                              renderer=base.MaterializeRenderer())
        print res



if __name__ == '__main__':
    unittest.main(verbosity=2)
