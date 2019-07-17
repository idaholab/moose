#!/usr/bin/env python3
import mock
import unittest
import logging
import collections
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, autolink, heading, sqa
from MooseDocs import base
from MooseDocs.tree import pages
logging.basicConfig()

class TestSQARequirementsAST(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testASTNoLink(self):
        text = u"!sqa requirements category=Demo link=False"
        ast = self.tokenize(text)

        self.assertSize(ast, 2)
        self.assertSize(ast(0), 5) # Tree 'demo' file
        self._assertAST_tree(ast(0), item_size=1)
        self._assertAST_common(ast(1), item_size=1)

    def testEmptyCategory(self):
        text = u"!sqa requirements category=_empty_"
        ast = self.tokenize(text)
        self.assertSize(ast, 0)

    def testASTSpecLink(self):
        text = u"!sqa requirements category=Demo link=True link-spec=True link-design=False link-issues=False link-prerequisites=False"
        ast = self.tokenize(text)

        self._assertAST_tree(ast(0), item_size=2)
        self._assertAST_common(ast(5), item_size=2)

        self.assertSize(ast, 14)
        for i in [1,2,3,4,6,7,8,9,10,11,12,13]:
            self.assertToken(ast(i), 'ModalLink')

        self.assertToken(ast(0)(1)(1), 'SQARequirementSpecification', spec_path='tree', spec_name='r0')
        self.assertToken(ast(0)(2)(1), 'SQARequirementSpecification', spec_path='tree', spec_name='r1')
        self.assertToken(ast(0)(3)(1), 'SQARequirementSpecification', spec_path='tree', spec_name='r2')
        self.assertToken(ast(0)(4)(1), 'SQARequirementSpecification', spec_path='tree', spec_name='r3')

        self.assertToken(ast(5)(1)(1), 'SQARequirementSpecification', spec_path='common', spec_name='r0')
        self.assertToken(ast(5)(2)(1), 'SQARequirementSpecification', spec_path='common', spec_name='r1')
        self.assertToken(ast(5)(3)(1), 'SQARequirementSpecification', spec_path='common', spec_name='r2')
        self.assertToken(ast(5)(4)(1), 'SQARequirementSpecification', spec_path='common', spec_name='r3')

        self.assertToken(ast(5)(5)(2), 'SQARequirementSpecification', spec_path='common', spec_name='group0')
        self.assertToken(ast(5)(6)(2), 'SQARequirementSpecification', spec_path='common', spec_name='group1')
        self.assertToken(ast(5)(7)(2), 'SQARequirementSpecification', spec_path='common', spec_name='group2')
        self.assertToken(ast(5)(8)(2), 'SQARequirementSpecification', spec_path='common', spec_name='group3')

    def testASTDesignLink(self):
        text = u"!sqa requirements category=Demo link=True link-spec=False link-design=True link-issues=False link-prerequisites=False"
        ast = self.tokenize(text)

        self._assertAST_tree(ast(0), item_size=2)
        self._assertAST_common(ast(1), item_size=2)

        self.assertToken(ast(0)(1)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(2)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(0)(3)(1), 'SQARequirementDesign', design=['bibtex.md'])
        self.assertToken(ast(0)(4)(1), 'SQARequirementDesign', design=['katex.md'])

        self.assertToken(ast(1)(1)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(1)(2)(1), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(1)(3)(1), 'SQARequirementDesign', design=['bibtex.md'])
        self.assertToken(ast(1)(4)(1), 'SQARequirementDesign', design=['katex.md'])

        self.assertToken(ast(1)(5)(2), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(1)(6)(2), 'SQARequirementDesign', design=['core.md'])
        self.assertToken(ast(1)(7)(2), 'SQARequirementDesign', design=['bibtex.md'])
        self.assertToken(ast(1)(8)(2), 'SQARequirementDesign', design=['katex.md'])

    def testASTIssuesLink(self):
        text = u"!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=True link-prerequisites=False"
        ast = self.tokenize(text)

        self._assertAST_tree(ast(0), item_size=2)
        self._assertAST_common(ast(1), item_size=2)

        self.assertToken(ast(0)(1)(1), 'SQARequirementIssues', issues=['#1980'])
        self.assertToken(ast(0)(2)(1), 'SQARequirementIssues', issues=['#2011'])
        self.assertToken(ast(0)(3)(1), 'SQARequirementIssues', issues=['#1980'])
        self.assertToken(ast(0)(4)(1), 'SQARequirementIssues', issues=['#2013'])

        self.assertToken(ast(1)(1)(1), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(1)(2)(1), 'SQARequirementIssues', issues=['#3456'])
        self.assertToken(ast(1)(3)(1), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(1)(4)(1), 'SQARequirementIssues', issues=['#4567'])

        self.assertToken(ast(1)(5)(2), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(1)(6)(2), 'SQARequirementIssues', issues=['#8910'])
        self.assertToken(ast(1)(7)(2), 'SQARequirementIssues', issues=['#1234'])
        self.assertToken(ast(1)(8)(2), 'SQARequirementIssues', issues=['#4321'])

    def testASTPrereqLink(self):
        text = u"!sqa requirements category=Demo link=True link-spec=False link-design=False link-issues=False link-prerequisites=True"
        ast = self.tokenize(text)

        self.assertToken(ast(0)(0), 'SQARequirementMatrixHeading', size=1)
        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=1)
        self.assertToken(ast(0)(2), 'SQARequirementMatrixItem', size=1)
        self.assertToken(ast(0)(3), 'SQARequirementMatrixItem', size=2)
        self.assertToken(ast(0)(4), 'SQARequirementMatrixItem', size=2)

        self.assertToken(ast(0)(3)(1), 'SQARequirementPrequisites', size=0, specs=[('tree', 'r1', 'F1.1.2')])
        self.assertToken(ast(0)(4)(1), 'SQARequirementPrequisites', size=0, specs=[('tree', 'r1', 'F1.1.2')])

        self._assertAST_common(ast(1), item_size=1)

    def _assertHTML(self, res):
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'p', size=5)

        self.assertHTMLTag(res(0)(0), 'span', string=u'Idaho National Laboratory (INL)')

        self.assertHTMLString(res(0)(1), u' ')
        self.assertHTMLString(res(0)(2), u'and')
        self.assertHTMLString(res(0)(3), u' ')

        self.assertHTMLTag(res(0)(4), 'span', string=u'INL')

    def _assertAST_tree(self, ast, item_size=None):
        """python/MooseDocs/tests/tree/demo"""

        self.assertToken(ast(0), 'SQARequirementMatrixHeading', category=u'Demo', string=u'Tree')

        for i, s in enumerate([u'One', u'Two', u'Three', u'Four']):
            self.assertToken(ast(i+1), 'SQARequirementMatrixItem', size=item_size)
            self.assertToken(ast(i+1)(0), 'SQARequirementText', size=3)
            self.assertToken(ast(i+1)(0)(0), 'Word', size=0, content=u'Tree')
            self.assertToken(ast(i+1)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+1)(0)(2), 'Word', size=0, content=s)

    def _assertAST_common(self, ast, item_size=None):
        """python/MooseDocs/tests/common/demo"""

        self.assertToken(ast(0), 'SQARequirementMatrixHeading', category=u'Demo', string=u'Common')

        for i, s in enumerate([u'One', u'Two', u'Three', u'Four']):
            self.assertToken(ast(i+1), 'SQARequirementMatrixItem', size=item_size)
            self.assertToken(ast(i+1)(0), 'SQARequirementText', size=3)
            self.assertToken(ast(i+1)(0)(0), 'Word', size=0, content=u'Requirement')
            self.assertToken(ast(i+1)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+1)(0)(2), 'Word', size=0, content=s)

        for i, s in enumerate([u'One', u'Two', u'Three', u'Four']):
            self.assertToken(ast(i+5), 'SQARequirementMatrixItem', size=item_size+1)
            self.assertToken(ast(i+5)(0), 'SQARequirementText', size=5)
            self.assertToken(ast(i+5)(0)(0), 'Word', size=0, content=u'Requirement')
            self.assertToken(ast(i+5)(0)(1), 'Space', size=0, count=1)
            self.assertToken(ast(i+5)(0)(2), 'Word', size=0, content=u'Group')
            self.assertToken(ast(i+5)(0)(3), 'Space', size=0, count=1)
            self.assertToken(ast(i+5)(0)(4), 'Word', size=0, content=s)

class TestSQAVerificationAndValidation(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))
    def testVerification(self):
        text = u"!sqa verification category=Demo"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=2)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixItem', size=5)
        self.assertToken(ast(0)(0)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(0)(2), 'Word', content='Four')

        self.assertToken(ast(0)(0)(3), 'Paragraph', size=2)
        self.assertToken(ast(0)(0)(3)(0), 'String', content=u'Specification: ')
        self.assertToken(ast(0)(0)(3)(1), 'Link', string=u'common:r3')

        self.assertToken(ast(0)(0)(4), 'Paragraph', size=2)
        self.assertToken(ast(0)(0)(4)(0), 'String', content=u'Documentation: ')
        self.assertToken(ast(0)(0)(4)(1), 'AutoLink', page='katex.md')

        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=7)
        self.assertToken(ast(0)(1)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(1)(1), 'Space', count=1)
        self.assertToken(ast(0)(1)(2), 'Word', content='Group')
        self.assertToken(ast(0)(1)(3), 'Space', count=1)
        self.assertToken(ast(0)(1)(4), 'Word', content='One')

        self.assertToken(ast(0)(1)(5), 'Paragraph', size=2)
        self.assertToken(ast(0)(1)(5)(0), 'String', content=u'Specification: ')
        self.assertToken(ast(0)(1)(5)(1), 'Link', string=u'common:group0')

        self.assertToken(ast(0)(1)(6), 'Paragraph', size=2)
        self.assertToken(ast(0)(1)(6)(0), 'String', content=u'Documentation: ')
        self.assertToken(ast(0)(1)(6)(1), 'AutoLink', page='bibtex.md')

        self.assertToken(ast(1), 'ModalLink', size=2)
        self.assertToken(ast(1)(0), 'ModalLinkTitle', size=1)
        self.assertIn(u'python/MooseDocs/test/common/demo', ast(1)(0)(0)['content'])
        self.assertToken(ast(1)(1), 'ModalLinkContent', size=1)
        self.assertToken(ast(1)(1)(0), 'Code', size=0)

    def testValidation(self):
        text = u"!sqa validation category=Demo"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=2)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixItem', size=5)
        self.assertToken(ast(0)(0)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(0)(2), 'Word', content='Three')

        self.assertToken(ast(0)(0)(3), 'Paragraph', size=2)
        self.assertToken(ast(0)(0)(3)(0), 'String', content=u'Specification: ')
        self.assertToken(ast(0)(0)(3)(1), 'Link', string=u'common:r2')

        self.assertToken(ast(0)(0)(4), 'Paragraph', size=2)
        self.assertToken(ast(0)(0)(4)(0), 'String', content=u'Documentation: ')
        self.assertToken(ast(0)(0)(4)(1), 'AutoLink', page='alert.md')

        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=7)
        self.assertToken(ast(0)(1)(0), 'Word', content='Requirement')
        self.assertToken(ast(0)(1)(1), 'Space', count=1)
        self.assertToken(ast(0)(1)(2), 'Word', content='Group')
        self.assertToken(ast(0)(1)(3), 'Space', count=1)
        self.assertToken(ast(0)(1)(4), 'Word', content='Three')

        self.assertToken(ast(0)(1)(5), 'Paragraph', size=2)
        self.assertToken(ast(0)(1)(5)(0), 'String', content=u'Specification: ')
        self.assertToken(ast(0)(1)(5)(1), 'Link', string=u'common:group2')

        self.assertToken(ast(0)(1)(6), 'Paragraph', size=2)
        self.assertToken(ast(0)(1)(6)(0), 'String', content=u'Documentation: ')
        self.assertToken(ast(0)(1)(6)(1), 'AutoLink', page='katex.md')

        self.assertToken(ast(1), 'ModalLink', size=2)
        self.assertToken(ast(1)(0), 'ModalLinkTitle', size=1)
        self.assertIn(u'python/MooseDocs/test/common/demo', ast(1)(0)(0)['content'])
        self.assertToken(ast(1)(1), 'ModalLinkContent', size=1)
        self.assertToken(ast(1)(1)(0), 'Code', size=0)

    def testEmptyCategory(self):
        text = u"!sqa verification category=_empty_"
        ast = self.tokenize(text)
        self.assertSize(ast, 0)

        text = u"!sqa validation category=_empty_"
        ast = self.tokenize(text)
        self.assertSize(ast, 0)


class TestSQARequirementsMatrix(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))
    def testCommand(self):
        text = u"!sqa requirements-matrix prefix=Z\n- One\n- Two"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=2)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'Paragraph', size=2)
        self.assertToken(ast(0)(0)(0)(0), 'Word', content=u'One')
        self.assertToken(ast(0)(0)(0)(1), 'Break', count=1)
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(1)(0)(0), 'Word', content=u'Two')


class TestSQARequirementsCrossReference(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]
    NodeProxy = collections.namedtuple('NodeProxy', 'local')

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    @mock.patch.object(base.translators.Translator, 'findPage', side_effect=lambda x: TestSQARequirementsCrossReference.NodeProxy(local=x))
    def testCommand(self, mock_find_page):
        text = u"!sqa cross-reference category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 15)
        self.assertToken(ast(0), 'SQARequirementMatrix', size=4)
        self.assertToken(ast(0)(0), 'SQARequirementMatrixHeading', category=u'Demo', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page=u'bibtex.md', size=0)
        self.assertToken(ast(0)(1), 'SQARequirementMatrixItem', size=4)

        self.assertToken(ast(0)(1)(0), 'SQARequirementText', size=3)
        self.assertToken(ast(0)(1)(0)(0), 'Word', content='Tree')
        self.assertToken(ast(0)(1)(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(1)(0)(2), 'Word', content='Three')

        self.assertToken(ast(0)(1)(1), 'SQARequirementSpecification', size=1)
        self.assertToken(ast(0)(1)(1)(0), 'Link', string='tree:r2')

        self.assertToken(ast(0)(1)(2), 'SQARequirementDesign', size=0, design=['bibtex.md'])
        self.assertToken(ast(0)(1)(3), 'SQARequirementIssues', size=0, issues=['#1980'])

    def testEmptyCategory(self):
        text = u"!sqa cross-reference category=_empty_"
        ast = self.tokenize(text)
        self.assertSize(ast, 0)

class TestSQADependencies(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo']),
                                        Demo2=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = u"!sqa dependencies suffix=foo category=Demo"
        ast = self.tokenize(text)

        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'UnorderedList', size=1)
        self.assertToken(ast(0)(0), 'ListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page=u'sqa/Demo2_foo.md', optional=True, warning=True)

    def testCategoryEmpty(self):
        text = u"!sqa dependencies suffix=foo category=_empty_"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'UnorderedList', size=2)
        self.assertToken(ast(0)(0), 'ListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page=u'sqa/Demo2_foo.md', optional=True, warning=True)
        self.assertToken(ast(0)(1), 'ListItem', size=1)
        self.assertToken(ast(0)(1)(0), 'AutoLink', page=u'sqa/Demo_foo.md', optional=True, warning=True)

class TestSQADependenciesWithConfig(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'],
                                                  dependencies=['Demo2']),
                                        Demo2=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = u"!sqa dependencies suffix=foo category=Demo"
        ast = self.tokenize(text)

        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'UnorderedList', size=1)
        self.assertToken(ast(0)(0), 'ListItem', size=1)
        self.assertToken(ast(0)(0)(0), 'AutoLink', page=u'sqa/Demo2_foo.md', optional=True, warning=True)

class TestSQADocument(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCommand(self):
        text = u"!sqa document suffix=foo category=Demo"
        ast = self.tokenize(text)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AutoLink', page=u'sqa/Demo_foo.md', optional=True, warning=True)

class TestSQARequirementsRender(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, autolink, heading, sqa]

    def setupExtension(self, ext):
        if ext == sqa:
            return dict(active=True,
                        categories=dict(Demo=dict(directories=['python/MooseDocs/test'],
                                                  specs=['demo'])))

    def testCompleteRender(self):
        text = u"!sqa requirements category=Demo link=False"
        _, res = self.execute(text, renderer=base.HTMLRenderer())

        self.assertHTMLTag(res(0), 'ul', size=4, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(0)(0), 'li', size=2)
        self.assertHTMLTag(res(0)(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(0)(0)(1), 'span', size=3, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(0)(0)(1)(0), u'Tree')
        self.assertHTMLString(res(0)(0)(1)(1), u' ')
        self.assertHTMLString(res(0)(0)(1)(2), u'One')

        self.assertHTMLTag(res(1), 'ul', size=8, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(1)(0), 'li', size=2)
        self.assertHTMLTag(res(1)(0)(0), 'span', string='F1.2.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(1)(0)(1), 'span', size=3, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(1)(0)(1)(0), u'Requirement')
        self.assertHTMLString(res(1)(0)(1)(1), u' ')
        self.assertHTMLString(res(1)(0)(1)(2), u'One')

        self.assertHTMLTag(res(1), 'ul', size=8, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(1)(0), 'li', size=2)
        self.assertHTMLTag(res(1)(0)(0), 'span', string='F1.2.1', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(1)(0)(1), 'span', size=3, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(1)(0)(1)(0), u'Requirement')
        self.assertHTMLString(res(1)(0)(1)(1), u' ')
        self.assertHTMLString(res(1)(0)(1)(2), u'One')

        self.assertHTMLTag(res(1), 'ul', size=8, class_='moose-sqa-requirements')
        self.assertHTMLTag(res(1)(5), 'li', size=2)
        self.assertHTMLTag(res(1)(5)(0), 'span', string='F1.2.6', class_='moose-sqa-requirement-number')
        self.assertHTMLTag(res(1)(5)(1), 'span', size=6, class_='moose-sqa-requirement-content')
        self.assertHTMLString(res(1)(5)(1)(0), u'Requirement')
        self.assertHTMLString(res(1)(5)(1)(1), u' ')
        self.assertHTMLString(res(1)(5)(1)(2), u'Group')
        self.assertHTMLString(res(1)(5)(1)(3), u' ')
        self.assertHTMLString(res(1)(5)(1)(4), u'Two')
        ol = res(1)(5)(1)(5)
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
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', id_='path:Foo', class_='moose-sqa-requirement-number tooltipped', **{'data-tooltip':u'path:Foo'})
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

        tok = sqa.SQARequirementMatrixItem(None, label='F1.1.1', satisfied=False)

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number tooltipped moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res(0)(0), 'span', string='F1.1.1', class_='moose-sqa-requirement-number moose-sqa-requirement-unsatisfied')

        res = self.render(tok, renderer=base.LatexRenderer())
        arg = res(0)['args'][0]
        self.assertLatex(arg(0), 'Command', 'textcolor', string='F1.1.1')
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

        tok = sqa.SQARequirementText(None, string=u'stuff')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', string=u'stuff')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', string=u'stuff')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', string=u'stuff')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertLatexString(res(0), u'stuff')

    @mock.patch.object(sqa.RenderSQARequirementDesign, 'findDesign')
    def testSQARequirementDesign(self, mock_design):
        """
        RenderSQARequirementDesign.findDesign tries to look up a page via the translator.findPage,
        but the current setup for unit testing is not setup to allow for page lookups, so this
        is just testing for the error case for now.

        TODO: MooseDocsTestCase needs to handle the ability to find pages
        """
        mock_design.return_value = None # Allows log error to be skipped
        tok = sqa.SQARequirementDesign(None, design=[u'file.md'], line=42, filename='file')

        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'file.md', class_='moose-sqa-error')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'file.md', class_='moose-sqa-error')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=2, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Design: ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'file.md', class_='moose-sqa-error')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexString(res(0), 'Design:~')
        self.assertLatex(res(1), 'Command', 'textcolor')
        self.assertLatexArg(res(1), 0, 'Brace', string='red')
        self.assertLatexString(res(1)(0), 'file.md')

    def testSQARequirementIssues(self):

        tok = sqa.SQARequirementIssues(None, issues=['#1', '#2'])
        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'#1', href=u'https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string=u'#2', href=u'https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'#1', href=u'https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string=u'#2', href=u'https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3, class_='moose-sqa-items')
        self.assertHTMLString(res(0)(0), u'Issue(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'#1', href=u'https://github.com/idaholab/moose/issues/1')
        self.assertHTMLTag(res(0)(2), 'a', string=u'#2', href=u'https://github.com/idaholab/moose/issues/2')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 3)
        self.assertLatexString(res(0), 'Issue(s):~')
        self.assertLatex(res(1), 'Command', 'href', string=u'#1')
        self.assertLatexArg(res(1), 0, 'Brace', u'https://github.com/idaholab/moose/issues/1')
        self.assertLatex(res(2), 'Command', 'href', string=u'#2')
        self.assertLatexArg(res(2), 0, 'Brace', u'https://github.com/idaholab/moose/issues/2')

    def testSQARequirementSpecification(self):

        tok = sqa.SQARequirementSpecification(None, spec_name='name', spec_path='path')
        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLString(res(0)(0), u'Specification: path:name')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLString(res(0)(0), u'Specification: ')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLString(res(0)(0), u'Specification: path:name')

        res = self.render(tok, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), 'Specification: path:name')

    def testSQARequirementPrequisites(self):
        tok = sqa.SQARequirementPrequisites(None, specs=[('path', 'name0', 'F1.1'),
                                                         ('path', 'name1', 'F1.2')])
        res = self.render(tok, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), u'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'F1.1', href=u'#path:name0')
        self.assertHTMLTag(res(0)(2), 'a', string=u'F1.2', href=u'#path:name1')

        res = self.render(tok, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), u'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'F1.1', href=u'#path:name0')
        self.assertHTMLTag(res(0)(2), 'a', string=u'F1.2', href=u'#path:name1')

        res = self.render(tok, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'p', size=3)
        self.assertHTMLString(res(0)(0), u'Prerequisite(s): ')
        self.assertHTMLTag(res(0)(1), 'a', string=u'F1.1', href=u'#path:name0')
        self.assertHTMLTag(res(0)(2), 'a', string=u'F1.2', href=u'#path:name1')

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


if __name__ == '__main__':
    unittest.main(verbosity=2)
