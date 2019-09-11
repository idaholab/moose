#!/usr/bin/env python
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
                         message="'/Kernels/NotAKernel' syntax was not recognized.")


class TestParameters(AppSyntaxTestCase):
    TEXT = "!syntax parameters /Kernels/Diffusion"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertEqual(ast(0).text(), 'Input Parameters')
        self.assertToken(ast(1), 'InputParametersToken')

        params = ast(1)['parameters']
        self.assertIsInstance(params, dict)
        self.assertIn('enable', params)
        self.assertIn('group_name', params['enable'])
        self.assertEqual(params['enable']['group_name'], 'Advanced')

        ast = self.tokenize('{} heading=None'.format(self.TEXT))
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'InputParametersToken')

        ast = self.tokenize(u"{} heading=Foo heading-level=3".format(self.TEXT))
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=1, level=3)
        self.assertEqual(ast(0).text(), 'Foo')
        self.assertToken(ast(1), 'InputParametersToken')

    def testHTML(self):
        _, res = self.execute(self.TEXT, renderer=base.HTMLRenderer())

        self.assertSize(res, 9)
        self.assertHTMLTag(res(0), 'h2', id_='input-parameters', size=3)
        self.assertEqual(res(0).text(), 'Input Parameters')

        self.assertHTMLTag(res(1), 'h3')
        self.assertEqual(res(1)['data-details-open'], 'open')
        self.assertEqual(res(1).text(), 'Required Parameters')

        self.assertHTMLTag(res(2), 'ul', size=1)
        self.assertHTMLTag(res(2,0), 'li', size=2)
        self.assertHTMLTag(res(2,0,0), 'strong', string='variable: ')
        self.assertHTMLTag(res(2,0,1), 'span')
        self.assertIn('The name of the variable', res(2,0,1,0)['content'])

        self.assertHTMLTag(res(3), 'h3')
        self.assertEqual(res(3)['data-details-open'], 'open')
        self.assertEqual(res(3).text(), 'Optional Parameters')

        self.assertHTMLTag(res(4), 'ul', size=2)

        self.assertHTMLTag(res(5), 'h3')
        self.assertEqual(res(5)['data-details-open'], 'close')
        self.assertEqual(res(5).text(), 'Advanced Parameters')

        self.assertHTMLTag(res(6), 'ul', size=7)

        self.assertHTMLTag(res(7), 'h3')
        self.assertEqual(res(7)['data-details-open'], 'close')
        self.assertEqual(res(7).text(), 'Tagging Parameters')

        self.assertHTMLTag(res(8), 'ul', size=4)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertSize(res, 9)
        self.assertHTMLTag(res(0), 'h2', id_='input-parameters', size=3)
        self.assertEqual(res(0).text(), 'Input Parameters')

        self.assertHTMLTag(res(1), 'h3')
        self.assertEqual(res(1)['data-details-open'], 'open')
        self.assertEqual(res(1).text(), 'Required Parameters')

        self.assertHTMLTag(res(2), 'ul', size=1, class_='collapsible')
        self.assertHTMLTag(res(2,0), 'li', size=2)
        self.assertHTMLTag(res(2,0,0), 'div', size=2, class_='collapsible-header')
        self.assertHTMLTag(res(2,0,0,0), 'span', class_='moose-parameter-name', string='variable')
        self.assertHTMLTag(res(2,0,0,1), 'span', class_='moose-parameter-header-description', size=1)
        self.assertIn('The name of the variable', res(2,0,0,1,0)['content'])

        self.assertHTMLTag(res(2,0,1), 'div', size=3, class_='collapsible-body')
        self.assertHTMLTag(res(2,0,1,0), 'p', size=2, class_='moose-parameter-description-cpptype')
        self.assertHTMLTag(res(2,0,1,1), 'p', size=2, class_='moose-parameter-description-options')
        self.assertHTMLTag(res(2,0,1,2), 'p', size=2, class_='moose-parameter-description')

        self.assertHTMLTag(res(3), 'h3')
        self.assertEqual(res(3)['data-details-open'], 'open')
        self.assertEqual(res(3).text(), 'Optional Parameters')

        self.assertHTMLTag(res(4), 'ul', size=2, class_='collapsible')

        self.assertHTMLTag(res(5), 'h3')
        self.assertEqual(res(5)['data-details-open'], 'close')
        self.assertEqual(res(5).text(), 'Advanced Parameters')

        self.assertHTMLTag(res(6), 'ul', size=7, class_='collapsible')

        self.assertHTMLTag(res(7), 'h3')
        self.assertEqual(res(7)['data-details-open'], 'close')
        self.assertEqual(res(7).text(), 'Tagging Parameters')

        self.assertHTMLTag(res(8), 'ul', size=4, class_='collapsible')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        print('\n',res)
        self.assertSize(res, 15)
        self.assertLatexCommand(res(0), 'chapter', size=4)
        self.assertLatexCommand(res(0,0), 'label', string=u'input-parameters')
        self.assertLatexString(res(0,1), content=u'Input')
        self.assertLatexString(res(0,2), content=u' ')
        self.assertLatexString(res(0,3), content=u'Parameters')

        self.assertLatex(res(1), 'Environment', 'InputParameter')
        self.assertLatexArg(res(1), 0, 'Brace')
        self.assertLatexArg(res(1), 1, 'Bracket')
        self.assertLatexArg(res(1), 2, 'Bracket')
        self.assertIn('The name of the variable', res(1,0)['content'])


class TestParam(AppSyntaxTestCase):
    TEXT = "[!param](/Kernels/Diffusion/variable)"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'ParameterToken', string=u'"variable"')

        param = ast(0,0)['parameter']
        self.assertEqual(param[u'basic_type'], u'String')
        self.assertEqual(param[u'cpp_type'], u'NonlinearVariableName')
        self.assertEqual(param[u'deprecated'], False)
        self.assertIn('The name of the variable', param['description'])
        self.assertEqual(param[u'group_name'], u'')
        self.assertEqual(param[u'options'], u'')
        self.assertEqual(param[u'required'], True)

    def testHTML(self):
        _, res = self.execute(self.TEXT, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLTag(res(0,0), 'span', string=u'"variable"', class_='moose-parameter-name')

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLTag(res(0,0), 'span', string=u'"variable"', class_='moose-parameter-name tooltipped')
        self.assertIn(u"The name of the variable", res(0,0)['data-tooltip'])

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexCommand(res(0), 'par')
        self.assertLatexString(res(1), content=u'"variable"')

class TestChildren(AppSyntaxTestCase):
    TEXT = "!syntax children /Kernels/Diffusion"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertToken(ast(0,0), 'Word', content=u'Child')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'Word', content=u'Objects')

        self.assertToken(ast(1), 'UnorderedList', class_='moose-list-children')
        self.assertToken(ast(1,0), 'ListItem', size=1)
        self.assertToken(ast(1,0,0), 'SyntaxLink', size=1)

        self.assertToken(ast(2), 'ModalLink')

class TestInputs(AppSyntaxTestCase):
    TEXT = "!syntax inputs /Kernels/Diffusion"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertToken(ast(0,0), 'Word', content=u'Input')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'Word', content=u'Files')

        self.assertToken(ast(1), 'UnorderedList', class_='moose-list-inputs')
        self.assertToken(ast(1,0), 'ListItem', size=1)
        self.assertToken(ast(1,0,0), 'SyntaxLink', size=1)

        self.assertToken(ast(2), 'ModalLink')

class TestComplete(AppSyntaxTestCase):
    TEXT = "!syntax complete"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertToken(ast(0), 'Heading', level=2, size=1)
        self.assertToken(ast(0,0), 'AutoLink', page=u'syntax/Adaptivity/index.md', string=u'Adaptivity')

        self.assertToken(ast(1), 'SyntaxList')
        self.assertToken(ast(1,0), 'SyntaxListItem', string=u'Moose App')

class TestList(AppSyntaxTestCase):
    TEXT = "!syntax list /Controls"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertToken(ast(0), 'Heading', size=11)
        self.assertEqual(ast(0).text(), "Available Objects Actions and Subsystems") # text() omits commas

        self.assertToken(ast(1), 'SyntaxList')
        self.assertToken(ast(1,0), 'SyntaxListItem', string=u'Moose App')


class TestRenderSyntaxList(AppSyntaxTestCase):

    @classmethod
    def setUpClass(cls):
        cls.AST = appsyntax.SyntaxList(None)
        appsyntax.SyntaxListItem(cls.AST, header=True, string=u'App', syntax=u'syntax', group=u'group')
        appsyntax.SyntaxListItem(cls.AST, string=u'item')

    def testHTML(self):
        res = self.render(self.AST, renderer=base.HTMLRenderer())
        #self.assertHTMLTag(res, 'body', size=1)
        #self.assertHTMLTag(res(0), 'div', size=2, class_='moose-syntax-list')
        #self.assertHTMLTag(res(0,'p'

    def testMaterialize(self):
        res = self.render(self.AST, renderer=base.MaterializeRenderer())


#SyntaxList = tokens.newToken('SyntaxList')
#SyntaxListItem = tokens.newToken('SyntaxListItem', syntax=u'', group=u'', header=False)
#SyntaxLink = tokens.newToken('SyntaxLink', core.Link)
if __name__ == '__main__':
    unittest.main(verbosity=2)
