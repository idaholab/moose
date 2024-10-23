#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mock
import unittest
import logging
import collections
import itertools
import moosesqa
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, autolink, heading, civet, sqa, table, modal
from MooseDocs import base
from MooseDocs.tree import pages
logging.basicConfig()

class TestSQARequirementsAST(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testASTNoLink(self):
        text = "!sqa requirements category=Demo link=False"
        ast = self.tokenize(text)
        self._assertAST_common(ast(0))

    def testASTSpecLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=True link-design=False link-issues=False link-prerequisites=False link-collections=False link-types=False"
        ast = self.tokenize(text)
        self._assertAST_common(ast(0))
        self._assertAST_tree(ast(1))

        for i in range(1, 4):
            self.assertToken(ast(0,i,1), 'Paragraph', size=2)
            self.assertToken(ast(0,i,1,0), 'String', content='Specification(s): ')
            self.assertToken(ast(0,i,1,1), 'ModalLink')
            self.assertEqual(ast(0,i,1,1)['content'].name, 'Code')

    def testASTDesignLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=False link-design=True link-issues=False link-prerequisites=False link-collections=False link-types=False"
        ast = self.tokenize(text)

        self._assertAST_common(ast(0))

        self.assertToken(ast(0)(1)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(2)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(3)(1), 'SQARequirementDesign', design=['bibtex.md'])
        self.assertToken(ast(0)(4)(1), 'SQARequirementDesign', design=['katex.md'])

        self.assertToken(ast(0)(5)(2), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(6)(2), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(7)(2), 'SQARequirementDesign', design=['bibtex.md'])
        self.assertToken(ast(0)(8)(2), 'SQARequirementDesign', design=['katex.md'])

    def testASTIssuesLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=True link-prerequisites=False link-collections=False link-types=False"
        ast = self.tokenize(text)

        self._assertAST_common(ast(0))

        self.assertToken(ast(0)(1)(1), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(0)(2)(1), 'SQARequirementIssues', issues=['#3456'])
        self.assertToken(ast(0)(3)(1), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(0)(4)(1), 'SQARequirementIssues', issues=['#4567'])

        self.assertToken(ast(0)(5)(2), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(0)(6)(2), 'SQARequirementIssues', issues=['#8910'])
        self.assertToken(ast(0)(7)(2), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(0)(8)(2), 'SQARequirementIssues', issues=['#4321'])

    def testASTPrereqLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=False link-prerequisites=True link-collections=False link-types=False link-results=False"
        ast = self.tokenize(text)
        self._assertAST_common(ast(0))
        self._assertAST_tree(ast(1))

        self.assertToken(ast(1)(3)(1), 'SQARequirementPrerequisites', size=0, specs=[('r1', '1.2.2')])
        self.assertToken(ast(1)(4)(1), 'SQARequirementPrerequisites', size=0, specs=[('r1', '1.2.2')])

        self.assertToken(ast(1)(0), 'SQARequirementMatrixHeading', size=1)
        self.assertToken(ast(1)(1), 'SQARequirementMatrixItem', size=1)
        self.assertToken(ast(1)(2), 'SQARequirementMatrixItem', size=1)
        self.assertToken(ast(1)(3), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(1)(4), 'SQARequirementMatrixItem', size=2)

    def testASTCollectionsLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=False link-prerequisites=False link-collections=True link-types=False"
        ast = self.tokenize(text)
        self._assertAST_common(ast(0))
        self._assertAST_tree(ast(1))
        self.assertToken(ast(0)(6), 'SQARequirementMatrixItem', size=6)
        self.assertToken(ast(0)(6)(2), 'SQARequirementCollections', size=0, collections={"Andrew"})

    def testASTTypesLink(self):
        text = "!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=False link-prerequisites=False link-collections=False link-types=True"
        ast = self.tokenize(text)
        self._assertAST_common(ast(0))
        self._assertAST_tree(ast(1))
        self.assertToken(ast(1)(3), 'SQARequirementMatrixItem', size=3)
        self.assertToken(ast(1)(3)(1), 'SQARequirementTypes', size=0, types={"TestType"})

    def _assertHTML(self, res):
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'p', size=5)

        self.assertHTMLTag(res(0)(0), 'span', string='Idaho National Laboratory (INL)')

        self.assertHTMLString(res(0)(1), ' ')
        self.assertHTMLString(res(0)(2), 'and')
        self.assertHTMLString(res(0)(3), ' ')

        self.assertHTMLTag(res(0)(4), 'span', string='INL')

    def _assertAST_tree(self, ast):
        """python/MooseDocs/tests/tree/demo"""

        self.assertToken(ast(0), 'SQARequirementMatrixHeading', category='Demo', string='Tree')

        for i, s in enumerate(['One', 'Two', 'Three', 'Four']):
            self.assertToken(ast(i+1), 'SQARequirementMatrixItem')
            self.assertToken(ast(i+1)(0), 'SQARequirementText', size=3)
            self.assertToken(ast(i+1)(0)(0), 'Word', size=0, content='Tree')
            self.assertToken(ast(i+1)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+1)(0)(2), 'Word', size=0, content=s)

    def _assertAST_common(self, ast):
        """python/MooseDocs/tests/common/demo"""
        self.assertToken(ast(0), 'SQARequirementMatrixHeading', category='Demo', string='Common')

        for i, s in enumerate(['One', 'Two', 'Three', 'Four']):
            self.assertToken(ast(i+1), 'SQARequirementMatrixItem')
            self.assertToken(ast(i+1)(0), 'SQARequirementText', size=3)
            self.assertToken(ast(i+1)(0)(0), 'Word', size=0, content='Requirement')
            self.assertToken(ast(i+1)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+1)(0)(2), 'Word', size=0, content=s)

        for i, s in enumerate(['One', 'Two', 'Three', 'Four']):
            self.assertToken(ast(i+5), 'SQARequirementMatrixItem')
            self.assertToken(ast(i+5)(0), 'SQARequirementText', size=5)
            self.assertToken(ast(i+5)(0)(0), 'Word', size=0, content='Requirement')
            self.assertToken(ast(i+5)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+5)(0)(2), 'Word', size=0, content='Group')
            self.assertToken(ast(i+5)(0)(3), 'Space', size=0, count=1)
            self.assertToken(ast(i+5)(0)(4), 'Word', size=0, content=s)

class TestSQAVerificationAndValidation(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))
    def testVerification(self):
        text = "!sqa verification category=Demo link-spec=false link-design=false link-issues=false link-results=false link-collections=false link-types=false"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=9)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixHeading', category='Demo')
        self.assertToken(ast(0)(3), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(0)(3)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(3)(0)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(3)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(3)(0)(2), 'Word', content='Three')
        self.assertToken(ast(0)(3)(1), 'Paragraph', size=3)
        self.assertToken(ast(0)(3)(1)(0), 'String', content='Verification: ')
        self.assertToken(ast(0)(3)(1)(1), 'AutoLink', page='sqa.md')

        self.assertToken(ast(0)(4), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(0)(4)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(4)(1), 'Paragraph', size=3)
        self.assertToken(ast(0)(4)(1)(0), 'String', content='Verification: ')
        self.assertToken(ast(0)(4)(1)(1), 'AutoLink', page='katex.md')

        self.assertToken(ast(0)(5), 'SQARequirementMatrixItem', size=3)
        self.assertToken(ast(0)(5)(0), 'SQARequirementText', size=5)
        self.assertToken(ast(0)(5)(1), 'SQARequirementDetails', size=2)
        self.assertToken(ast(0)(5)(2), 'Paragraph', size=3)
        self.assertToken(ast(0)(5)(2)(0), 'String', content='Verification: ')
        self.assertToken(ast(0)(5)(2)(1), 'AutoLink', page='bibtex.md')

    def testValidation(self):
        text = "!sqa validation category=Demo link-spec=false link-design=false link-issues=false link-results=false link-collections=false link-types=false"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=9)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixHeading', category='Demo')
        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(0)(1)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(1)(0)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(1)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(1)(0)(2), 'Word', content='One')

        self.assertToken(ast(0)(1)(1), 'Paragraph', size=3)
        self.assertToken(ast(0)(1)(1)(0), 'String', content='Validation: ')
        self.assertToken(ast(0)(1)(1)(1), 'AutoLink', page='special.md')

        self.assertToken(ast(0)(3), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(0)(3)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(3)(1), 'Paragraph', size=3)
        self.assertToken(ast(0)(3)(1)(0), 'String', content='Validation: ')
        self.assertToken(ast(0)(3)(1)(1), 'AutoLink', page='alert.md')

        self.assertToken(ast(0)(7), 'SQARequirementMatrixItem', size=3)
        self.assertToken(ast(0)(7)(0), 'SQARequirementText', size=5)
        self.assertToken(ast(0)(7)(1), 'SQARequirementDetails', size=2)
        self.assertToken(ast(0)(7)(2), 'Paragraph', size=5)
        self.assertToken(ast(0)(7)(2)(0), 'String', content='Validation: ')
        self.assertToken(ast(0)(7)(2)(1), 'AutoLink', page='katex.md')
        self.assertToken(ast(0)(7)(2)(3), 'AutoLink', page='bibtex.md')

class TestSQARequirementsCrossReference(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]
    NodeProxy = collections.namedtuple('NodeProxy', 'local')

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    @mock.patch.object(base.Translator, 'findPage', side_effect=lambda x: TestSQARequirementsCrossReference.NodeProxy(local=x))
    def testCommand(self, mock_find_page):
        text = "!sqa cross-reference category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 3)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=7)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixHeading', category='Demo', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page='core.md', size=0)
        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=9)

        self.assertToken(ast(0)(1)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(1)(0)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(1)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(1)(0)(2), 'Word', content='One')

        self.assertToken(ast(0)(1)(1), 'Paragraph', size=2)
        self.assertToken(ast(0)(1)(1)(0), 'String', content='Specification(s): ')
        self.assertToken(ast(0)(1)(1)(1), 'ModalLink', size=1, string='r0')

        self.assertToken(ast(0)(1)(2), 'SQARequirementDesign', size=0, design=['core.md'])
        self.assertToken(ast(0)(1)(3), 'SQARequirementIssues', size=0, issues=['#1234'])

class TestSQADependencies(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo']),
                                        Demo2=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = "!sqa dependencies suffix=foo category=Demo"
        ast = self.tokenize(text)

        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'UnorderedList', size=1)
        self.assertToken(ast(0)(0), 'ListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page='sqa/Demo2_foo.md', optional=True, warning=True)

class TestSQADependenciesWithConfig(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'],
                                                  dependencies=['Demo2']),
                                        Demo2=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = "!sqa dependencies suffix=foo category=Demo"
        ast = self.tokenize(text)

        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'UnorderedList', size=1)
        self.assertToken(ast(0)(0), 'ListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page='sqa/Demo2_foo.md', optional=True, warning=True)

class TestSQADocument(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = "!sqa document suffix=foo category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AutoLink', page='sqa/Demo_foo.md', optional=True, warning=True)

class TestSQARequirementsRender(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCompleteRender(self):
        text = "!sqa requirements category=Demo link=False"
        _, res = self.execute(text, renderer=base.HTMLRenderer())

        self.assertHTMLTag(res(0), 'ul', size=8, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(0)(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0)(0), 'span', string='1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(0)(1), 'span', size=3, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(0)(0)(1)(0), 'Requirement')
        self.assertHTMLString(res(0)(0)(1)(1), ' ')
        self.assertHTMLString(res(0)(0)(1)(2), 'One')

        self.assertHTMLTag(res(0), 'ul', size=8, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(0)(5), 'li', size=2)
        self.assertHTMLTag(res(0)(5)(0), 'span', string='1.1.6', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(5)(1), 'span', size=6, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(0)(5)(1)(0), 'Requirement')
        self.assertHTMLString(res(0)(5)(1)(1), ' ')
        self.assertHTMLString(res(0)(5)(1)(2), 'Group')
        self.assertHTMLString(res(0)(5)(1)(3), ' ')
        self.assertHTMLString(res(0)(5)(1)(4), 'Two')

        ol = res(0)(5)(1)(5)
        self.assertHTMLTag(ol, 'ol', size=2, class_='moose-sqa-details-list')
        self.assertHTMLTag(ol(0), 'li', size=2, class_='moose-sqa-detail-item')
        self.assertHTMLString(ol(0)(0), '3')
        self.assertHTMLString(ol(0)(1), 'D')

    def testSQARequirementMatrix(self):
        tok = sqa.SQARequirementMatrix(None)

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'ul', size=0, class_='moose-sqa-requirements')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'ul', size=0, class_='moose-sqa-requirements collection with-header')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'ul', size=0, class_='moose-sqa-requirements')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 0)

    def testSQARequirementMatrixItemSatisfied(self):

        tok = sqa.SQARequirementMatrixItem(None, label='F1.1.1', reqname='path:Foo')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'li', size=2, class_='collection-item')
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', id_='path:Foo', class_='moose-sqa-requirement-number tooltipped', **{'data-tooltip':'path:Foo'})
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'Requirement')
        self.assertLatexArg(res(0), 0, 'Brace', string='F1.1.1')


    def testSQARequirementMatrixItemUnSatisfied(self):

        tok = sqa.SQARequirementMatrixItem(None, label='1.1.1', satisfied=False)

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='1.1.1', class_='moose-sqa-requirement-number moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='1.1.1', class_='moose-sqa-requirement-number tooltipped moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='1.1.1', class_='moose-sqa-requirement-number moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.LatexRenderer())
        arg = res(0)['args'][0]
        self.assertLatex(arg(0), 'Command', 'textcolor', string='1.1.1')
        self.assertLatexArg(arg(0), 0, 'Brace', string='red')

    def testSQARequirementMatrixListItem(self):

        tok = sqa.SQARequirementMatrixListItem(None, label='F1.1.1')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'li', size=2, class_='collection-item')
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(1), 'span', size=0, class_='moose-sqa-requirement-content')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'Requirement')
        self.assertLatexArg(res(0), 0, 'Brace', string='F1.1.1')

    def testSQARequirementText(self):

        tok = sqa.SQARequirementText(None, string='stuff')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', string='stuff')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', string='stuff')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', string='stuff')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertLatexString(res(0), 'stuff')

    @mock.patch.object(sqa.RenderSQARequirementDesign, 'findDesign')
    def testSQARequirementDesign(self, mock_design):
        """
        RenderSQARequirementDesign.findDesign tries to look up a page via the translator.findPage,
        but the current setup for unit testing is not setup to allow for page lookups, so this
        is just testing for the error case for now.

        TODO: MooseDocsTestCase needs to handle the ability to find pages
        """
        mock_design.return_value = None # Allows log error to be skipped
        tok = sqa.SQARequirementDesign(None, design=['file.md'], line=42, filename='file')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string='file.md', class_='moose-error')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string='file.md', class_='moose-error')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string='file.md', class_='moose-error')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexString(res(0), 'Design:~')
        self.assertLatex(res(1), 'Command', 'textcolor')
        self.assertLatexArg(res(1), 0, 'Brace', string='red')
        self.assertLatexString(res(1)(0), 'file.md')

    def testSQARequirementIssues(self):

        tok = sqa.SQARequirementIssues(None, issues=['#1', '#2'], url='https://github.com/idaholab/moose')
        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='#1', href='https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string='#2', href='https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='#1', href='https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string='#2', href='https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), 'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='#1', href='https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string='#2', href='https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 3)
        self.assertLatexString(res(0), 'Issue(s):~')
        self.assertLatex(res(1), 'Command', 'href', string='#1')
        self.assertLatexArg(res(1), 0, 'Brace', 'https://github.com/idaholab/moose/issues/1')
        self.assertLatex(res(2), 'Command', 'href', string='#2')
        self.assertLatexArg(res(2), 0, 'Brace', 'https://github.com/idaholab/moose/issues/2')

    def testSQARequirementPrerequisites(self):
        tok = sqa.SQARequirementPrerequisites(None, specs=[('name0', 'F1.1'),
                                                           ('name1', 'F1.2')])
        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), 'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='F1.1', href='#name0')
        self.assertHTMLTag(res(0)(2), 'a', string='F1.2', href='#name1')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), 'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='F1.1', href='#name0')
        self.assertHTMLTag(res(0)(2), 'a', string='F1.2', href='#name1')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), 'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string='F1.1', href='#name0')
        self.assertHTMLTag(res(0)(2), 'a', string='F1.2', href='#name1')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertLatexString(res(0), 'Prerequisite(s):~')
        self.assertLatexString(res(1), 'F1.1; F1.2')

    def testSQARequirementDetails(self):

        tok = sqa.SQARequirementDetails(None)
        sqa.SQARequirementDetailItem(tok, label='foo')
        sqa.SQARequirementDetailItem(tok, label='bar')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'ol', size=2, class_='moose-sqa-details-list')
        self.assertHTMLTag(res(0)(0), 'li', size=0, class_='moose-sqa-detail-item')
        self.assertHTMLTag(res(0)(1), 'li', size=0, class_='moose-sqa-detail-item')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'ol', size=2, class_='moose-sqa-details-list')
        self.assertHTMLTag(res(0)(0), 'li', size=0, class_='moose-sqa-detail-item')
        self.assertHTMLTag(res(0)(1), 'li', size=0, class_='moose-sqa-detail-item')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'ol', size=2, class_='moose-sqa-details-list')
        self.assertHTMLTag(res(0)(0), 'li', size=0, class_='moose-sqa-detail-item')
        self.assertHTMLTag(res(0)(1), 'li', size=0, class_='moose-sqa-detail-item')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertLatex(res(0), 'Environment', 'enumerate')
        self.assertLatex(res(0)(0), 'Command', 'item')
        self.assertLatex(res(0)(1), 'Command', 'item')

    def testSQARequirementMatrixHeading(self):

        root = sqa.SQARequirementMatrix(None)
        sqa.SQARequirementMatrixItem(root, label='F1.1.1')
        tok = sqa.SQARequirementMatrixHeading(root, category='MOOSE')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertSize(res, 0)

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'li', string='MOOSE: ', class_='collection-header')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertSize(res, 0)

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Command', 'section*', string='F1:~')

class TestSQARequiremetnsWithCollectionsAndTypesAST(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, civet, sqa, table, modal]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCollections(self):
        text = "!sqa requirements collections=Andrew Deanne link=False category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'SQARequirementMatrix')
        self.assertSize(ast(0), 3)
        self.assertToken(ast(0,0), 'SQARequirementMatrixHeading', string='Common')
        self.assertToken(ast(0,1), 'SQARequirementMatrixItem', reqname='group1')
        self.assertToken(ast(0,2), 'SQARequirementMatrixItem', reqname='group2')

    def testTypes(self):
        text = "!sqa requirements types=TestType link=False category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'SQARequirementMatrix')
        self.assertSize(ast(0), 3)
        self.assertToken(ast(0,0), 'SQARequirementMatrixHeading', string='Tree')
        self.assertToken(ast(0,1), 'SQARequirementMatrixItem', reqname='r0')
        self.assertToken(ast(0,2), 'SQARequirementMatrixItem', reqname='r2')

class TestSQARegex(unittest.TestCase):
    def testIssue(self):
        regex = sqa.RenderSQARequirementIssues.ISSUE_RE
        match = regex.search('#12345')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), None)
        self.assertEqual(match.group('issues'), '12345')

        match = regex.search('moose#12345')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), 'moose')
        self.assertEqual(match.group('issues'), '12345')

        match = regex.search('some-app#12345')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), 'some-app')
        self.assertEqual(match.group('issues'), '12345')

        self.assertIsNone(regex.search('#wrong'))

    def testCommit(self):
        regex = sqa.RenderSQARequirementIssues.COMMIT_RE
        match = regex.search('a1b2c3d4e5')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), None)
        self.assertEqual(match.group('commit'), 'a1b2c3d4e5')

        match = regex.search('moose:a1b2c3d4e5')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), 'moose')
        self.assertEqual(match.group('commit'), 'a1b2c3d4e5')

        match = regex.search('some-app:a1b2c3d4e5')
        self.assertIsNotNone(match)
        self.assertEqual(match.group('key'), 'some-app')
        self.assertEqual(match.group('commit'), 'a1b2c3d4e5')

        self.assertIsNone(regex.search('wrong'))
        self.assertIsNone(regex.search('abc123')) # must be at least 10

if __name__ == '__main__':
    unittest.main(verbosity=2)
