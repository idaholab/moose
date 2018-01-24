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

import unittest
import bs4

import MooseDocs
from MooseDocs.common import moose_docs_file_tree
from MooseDocs.testing import MarkdownTestCase

class TestTemplate(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.template', 'MooseDocs.extensions.app_syntax', 'meta']

    @classmethod
    def updateExtensions(cls, configs):
        """
        Method to change the arguments that come from the configuration file for
        specific tests.  This way one can test optional arguments without permanently
        changing the configuration file.
        """
        configs['MooseDocs.extensions.template']['template'] = 'testing.html'
        configs['MooseDocs.extensions.app_syntax']['hide']['framework'].append('/Functions')
        configs['MooseDocs.extensions.app_syntax']['hide']['phase_field'].append('/ICs')

    @classmethod
    def setUpClass(cls):
        super(TestTemplate, cls).setUpClass()

        # Use BoxMarker.md to test Doxygen and Code lookups
        config = dict(base='docs/content',
                      include=['docs/content/documentation/systems/Adaptivity/Markers/**'])
        root = moose_docs_file_tree({'framework': config})
        node = root.findall('/BoxMarker')[0]
        cls.html = cls.parser.convert(node)

        #with open(node.markdown(), 'r') as fid:
        #    cls.html = fid.read()
        cls.soup = bs4.BeautifulSoup(cls.html, "html.parser")

    def testContent(self):
        self.assertIsNotNone(self.soup.find('h1'))
        self.assertIn('BoxMarker', self.html)

    def testDoxygen(self):
        a = str(self.soup)
        self.assertIsNotNone(a)
        self.assertIn('classBoxMarker.html', a)
        self.assertIn('Doxygen', a)

    def testCode(self):
        html = str(self.soup)
        self.assertIn('href="https://github.com/idaholab/moose/blob/master/framework/include/'\
                      'markers/BoxMarker.h"', html)
        self.assertIn('href="https://github.com/idaholab/moose/blob/master/framework/src/'\
                      'markers/BoxMarker.C"', html)

    def testHidden(self):
        md = '!syntax objects /Functions'
        html = self.convert(md)
        gold = '<a class="moose-bad-link" data-moose-disable-link-error="1" ' \
               'href="/Functions/framework/ParsedVectorFunction.md">ParsedVectorFunction</a>'
        self.assertIn(gold.format(MooseDocs.MOOSE_DIR.rstrip('/')), html)

    def testPolycrystalICs(self):
        md = '[Foo](/ICs/PolycrystalICs/index.md)'
        html = self.convert(md)
        gold = '<a class="moose-bad-link" href="/ICs/PolycrystalICs/index.md">'
        self.assertIn(gold, html)

class TestTemplateDisplay(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.template', 'MooseDocs.extensions.app_syntax', 'meta']

    def testDisplayName(self):
        config = dict(base='docs/content',
                      include=['docs/content/documentation/modules/phase_field/MultiPhase/**'])
        root = moose_docs_file_tree({'framework': config})
        node = root.findall('/KKSMultiComponentExample')[0]
        html = self.parser.convert(node)
        self.assertIn('<a class="breadcrumb" href="index.html">KKSMulti Component Example</a>',
                      html)



if __name__ == '__main__':
    unittest.main(verbosity=2)
