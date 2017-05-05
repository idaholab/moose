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

import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestBibtexExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.bibtex']

    @classmethod
    def updateExtensions(cls, configs):
        """
        Method to change the arguments that come from the configuration file for
        specific tests.  This way one can test optional arguments without permanently
        changing the configuration file.
        """
        configs['MooseDocs.extensions.bibtex']['macro_files'] =\
            ['docs/bib/macro_test_abbrev.bib']

    def testCite(self):
        md = r'\cite{testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_cite.html', md)

    def testCiteTwo(self):
        md = r'\cite{testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citeTwo.html', md)

    def testCiteThree(self):
        md = r'\cite{testkey, testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citeThree.html', md)

    def testCitet(self):
        md = r'\citet{testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citet.html', md)

    def testCitetTwo(self):
        md = r'\citet{testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citetTwo.html', md)

    def testCitetThree(self):
        md = r'\citet{testkey, testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citetThree.html', md)

    def testCitep(self):
        md = r'\citep{testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citep.html', md)

    def testCitepTwo(self):
        md = r'\citep{testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citepTwo.html', md)

    def testCitepThree(self):
        md = r'\citep{testkey, testkey, testkey}\n\bibliography{docs/bib/moose.bib}'
        self.assertConvert('test_citepThree.html', md)

    def testBibtexMacro(self):
        md = r'\cite{macroTestKey}\n\bibliography{docs/bib/macro_test.bib}'
        self.assertConvert('test_bibtex_macro.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
