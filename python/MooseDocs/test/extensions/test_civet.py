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
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, civet
from MooseDocs.tree import pages
from MooseDocs import base
logging.basicConfig()

@unittest.skip("Disabled to avoid excessive network access")
class CivetTestCase(MooseDocsTestCase):
    def assertURL(self, node):
        url = node['url']
        self.assertIn('https://civet.inl.gov/sha_events/idaholab/moose/', url)
        sha = url[48:]
        self.assertEqual(len(sha), 40)

class TestInlineCivet(CivetTestCase):
    """Test the the locally supplied url and repo are working. The code for this is shared so it
       should be good enough to test it with one command."""

    EXTENSIONS = [core, command, civet]
    RESULTS = "[!civet!results url=https://civet.inl.gov repo=idaholab/moose](Results)"
    RESULTS2 = "[!civet!results url=https://civet.inl.gov repo=idaholab/moose]"

    def setupExtension(self, ext):
        if ext == civet:
            return dict(generate_test_reports=False, test_results_cache='/tmp/civet/jobs')

    def testResults(self):
        ast = self.tokenize(self.RESULTS)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1)
        self.assertToken(ast(0,0,0), 'Word', size=0, content='Results')
        self.assertURL(ast(0,0))

    def testResults2(self):
        ast = self.tokenize(self.RESULTS2)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1)
        self.assertToken(ast(0,0,0), 'String', size=0)
        self.assertURL(ast(0,0))

class TestInlineCivetWithConfig(CivetTestCase):
    EXTENSIONS = [core, command, civet]
    RESULTS = "[!civet!results](Results)"
    RESULTS2 = "[!civet!results]"
    MERGERESULTS = "[!civet!mergeresults]"
    BADGES = "[!civet!badges tests=kernels/simple_diffusion.test]"
    REPORT = "!civet report tests=kernels/simple_diffusion.test"

    def setupExtension(self, ext):
        if ext == civet:
            return dict(generate_test_reports=False,
                        test_results_cache='/tmp/civet/jobs',
                        remotes=dict(moose=dict(url='https://civet.inl.gov',
                                                repo='idaholab/moose')))

    def testResultsAST(self):
        """!civet results with content; no need to render b/c it only uses core tokens"""
        ast = self.tokenize(self.RESULTS)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1)
        self.assertToken(ast(0,0,0), 'Word', size=0, content='Results')
        self.assertURL(ast(0,0))

    def testResultsAST2(self):
        """!civet results without content; no need to render b/c it only uses core tokens"""
        ast = self.tokenize(self.RESULTS2)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1)
        self.assertToken(ast(0,0,0), 'String', size=0)
        self.assertURL(ast(0,0))

    def testMergeResults(self):
        """!civet mergeresults; no need to render b/c it only uses core tokens"""

        ast = self.tokenize(self.MERGERESULTS)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=4)
        self.assertToken(ast(0,0), 'Link', size=1)
        self.assertToken(ast(0,0,0), 'String', size=0)
        self.assertToken(ast(0,1), 'LineBreak', size=0)

        self.assertURL(ast(0,0))
        self.assertToken(ast(0,2), 'Link', size=1)
        self.assertToken(ast(0,2,0), 'String', size=0)
        self.assertURL(ast(0,2))
        self.assertToken(ast(0,3), 'LineBreak', size=0)

    def testBadgesAST(self):
        ast = self.tokenize(self.BADGES)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'CivetTestBadges', size=0, tests=['kernels/simple_diffusion.test'])

    def testBadgesMaterialize(self):
        ast = self.tokenize(self.BADGES)
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertSize(res, 1)
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'p', size=1)
        self.assertHTMLTag(res(0,0), 'div', size=1, class_='moose-civet-badges')
        self.assertHTMLTag(res(0,0,0), 'span', size=1)
        self.assertHTMLTag(res(0,0,0,0), 'span', size=1, class_='new badge')
        self.assertIn('data-badge-caption', res(0,0,0,0))
        self.assertIn('data-status', res(0,0,0,0))

    def testReportAST(self):
        ast = self.tokenize(self.REPORT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'CivetTestReport', size=0, tests=['kernels/simple_diffusion.test'])

    def testReportMaterialize(self):
        ast = self.tokenize(self.REPORT)
        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='moose-civet-test-report')
        self.assertHTMLTag(res(0,0), 'table')
        self.assertGreater(len(res(0,0)), 1)

if __name__ == '__main__':
    unittest.main(verbosity=2)
