#!/usr/bin/env python3
import os
import unittest
import logging
import moosesyntax
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, table, floats, materialicon, autolink, heading, appsyntax, modal, alert
from MooseDocs import base
logging.basicConfig()

class AppSyntaxTestCase(MooseDocsTestCase):
    EXTENSIONS = [core, command, table, floats, materialicon, autolink, heading, appsyntax, modal, alert]

    def setupExtension(self, ext):
        if ext is appsyntax:
            return dict(executable=os.path.join(os.getenv('MOOSE_DIR'), 'test'))

class TestExternalPage(AppSyntaxTestCase):
    def testExternalPage(self):
        self._MooseDocsTestCase__text.external = True
        ast = self.tokenize(u"!syntax description /Kernels/Missing")
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='warning')
        self.assertToken(ast(0,0),'AlertTitle', size=1, brand='warning', icon_name='feedback', icon=True, prefix=True,
                         string='Disabled Object Syntax')
        self.assertToken(ast(0,1),'AlertContent', size=1)
        self.assertIn("This page is included from an external application", ast(0,1,0)['content'])

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
        enable_param = dict()
        for param in params:
            self.assertToken(param, 'ParameterToken')
            self.assertIsInstance(param['parameter'], dict)
            self.assertIn('name', param['parameter'])
            if param['parameter']['name'] == 'enable':
                enable_param = param['parameter']
                break
        self.assertIn('group_name', enable_param)
        self.assertEqual(enable_param['group_name'], 'Advanced')

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

        # This size should match the number of optional parameters for Kernel
        self.assertHTMLTag(res(4), 'ul', size=3)

        self.assertHTMLTag(res(5), 'h3')
        self.assertEqual(res(5)['data-details-open'], 'close')
        self.assertEqual(res(5).text(), 'Tagging Parameters')

        self.assertHTMLTag(res(6), 'ul', size=5)

        self.assertHTMLTag(res(7), 'h3')
        self.assertEqual(res(7)['data-details-open'], 'close')
        self.assertEqual(res(7).text(), 'Advanced Parameters')

        self.assertHTMLTag(res(8), 'ul', size=7)

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
        self.assertHTMLTag(res(2,0,1,1), 'p', size=2, class_='moose-parameter-description-controllable')
        self.assertHTMLTag(res(2,0,1,2), 'p', size=2, class_='moose-parameter-description')

        self.assertHTMLTag(res(3), 'h3')
        self.assertEqual(res(3)['data-details-open'], 'open')
        self.assertEqual(res(3).text(), 'Optional Parameters')

        # This size should match the number of optional parameters for Kernel
        self.assertHTMLTag(res(4), 'ul', size=3, class_='collapsible')

        self.assertHTMLTag(res(5), 'h3')
        self.assertEqual(res(5)['data-details-open'], 'close')
        self.assertEqual(res(5).text(), 'Tagging Parameters')

        self.assertHTMLTag(res(6), 'ul', size=5, class_='collapsible')

        self.assertHTMLTag(res(7), 'h3')
        self.assertEqual(res(7)['data-details-open'], 'close')
        self.assertEqual(res(7).text(), 'Advanced Parameters')

        self.assertHTMLTag(res(8), 'ul', size=7, class_='collapsible')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        # This size should correspond to the total number of parameters for
        # Diffusion (Required + Optional + Advanced + Tagging) + 1
        # (corresponding to 'type')
        self.assertSize(res, 17)
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
        self.assertToken(ast(0,0), 'ModalLink', string=u'"variable"')

        param = ast(0,0)['content']['parameter']
        self.assertEqual(param[u'basic_type'], u'String')
        self.assertEqual(param[u'cpp_type'], u'NonlinearVariableName')
        self.assertEqual(param[u'deprecated'], False)
        self.assertIn('The name of the variable', param['description'])
        self.assertEqual(param[u'group_name'], u'')
        self.assertEqual(param[u'options'], u'')
        self.assertEqual(param[u'required'], True)
        self.assertEqual(param[u'controllable'], False)

    def testHTML(self):
        _, res = self.execute(self.TEXT, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLTag(res(0,0), 'span', string=u'"variable"', class_='moose-modal-link')

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=2)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLTag(res(0,0), 'a', string=u'"variable"', class_='moose-modal-link modal-trigger')

        self.assertHTMLTag(res(1), 'div', size=1, class_='moose-modal modal')
        self.assertHTMLTag(res(1, 0), 'div', size=4, class_='modal-content')
        self.assertHTMLTag(res(1, 0, 0), 'h4', size=1, string=u'variable')
        self.assertHTMLTag(res(1, 0, 1), 'p', size=2, class_='moose-parameter-description-cpptype')
        self.assertEqual(u"NonlinearVariableName", res(1, 0, 1, 1)['content'])
        self.assertHTMLTag(res(1, 0, 2), 'p', size=2, class_='moose-parameter-description-controllable')
        self.assertHTMLString(res(1, 0, 2, 1), 'No')
        self.assertHTMLTag(res(1, 0, 3), 'p', size=2, class_='moose-parameter-description')
        self.assertIn(u"The name of the variable", res(1, 0, 3, 1)['content'])

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexCommand(res(0), 'par')
        self.assertLatexString(res(1), content=u'"variable"')

    def testMissing(self):
        ast = self.tokenize('[!param](/Kernels/Diffusion/foobar)')
        self.assertToken(ast(0,0), "ErrorToken")
        message = ast(0,0)['message']
        self.assertIn("Unable to locate the parameter '/Kernels/Diffusion/foobar', did you mean:", message)
        self.assertIn('    /Kernels/Diffusion/', message)

class TestChildren(AppSyntaxTestCase):
    TEXT = "!syntax children /Kernels/Diffusion"

    def testAST(self):
        ast = self.tokenize(self.TEXT)

        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertToken(ast(0,0), 'Word', content=u'Child')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'Word', content=u'Objects')

        self.assertToken(ast(1), 'UnorderedList', class_='moose-list-children')
        self.assertToken(ast(1,0), 'ListItem', size=1)
        self.assertToken(ast(1,0,0), 'ModalSourceLink', size=0)

class TestInputs(AppSyntaxTestCase):
    TEXT = "!syntax inputs /Kernels/Diffusion"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Heading', size=3, level=2)
        self.assertToken(ast(0,0), 'Word', content=u'Input')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'Word', content=u'Files')

        self.assertToken(ast(1), 'UnorderedList', class_='moose-list-inputs')
        self.assertToken(ast(1,0), 'ListItem', size=1)
        self.assertToken(ast(1,0,0), 'ModalSourceLink', size=0)

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

class TestSyntaxFailure(AppSyntaxTestCase):
    def test(self):
        moosesyntax.get_moose_syntax_tree = lambda *args: None # break syntax return
        with self.assertLogs(level=logging.ERROR) as cm:
            self._MooseDocsTestCase__setup()

        self.assertEqual(len(cm.output), 1)
        self.assertIn('Failed to load application executable', cm.output[0])
        self.assertIn('--json --allow-test-objects', cm.output[0])

if __name__ == '__main__':
    unittest.main(verbosity=2)
