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

import unittest

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
               ['docs/content/bib/macro_test_abbrev.bib']

    def testCite(self):
        md = r'\cite{testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_cite.html', md)

    def testCiteTwo(self):
        md = r'\cite{testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citeTwo.html', md)

    def testCiteThree(self):
        md = r'\cite{testkey, testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citeThree.html', md)

    def testCitet(self):
        md = r'\citet{testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citet.html', md)

    def testCitetTwo(self):
        md = r'\citet{testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citetTwo.html', md)

    def testCitetThree(self):
        md = r'\citet{testkey, testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citetThree.html', md)

    def testCitep(self):
        md = r'\citep{testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citep.html', md)

    def testCitepTwo(self):
        md = r'\citep{testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citepTwo.html', md)

    def testCitepThree(self):
        md = r'\citep{testkey, testkey, testkey}\n\bibliography{docs/content/bib/moose.bib}'
        self.assertConvert('test_citepThree.html', md)

    def testBibtexMacro(self):
        md = r'\cite{macroTestKey}\n\bibliography{docs/content/bib/test.bib}'
        self.assertConvert('test_bibtex_macro.html', md)

    def testNoAuthor(self):
        md = r'\cite{noAuthorTestKey}\n\bibliography{docs/content/bib/test.bib}'
        self.assertConvert('test_no_author.html', md)

    def testDuplicateError(self):
        md = r'\cite{macroTestKey}\n\bibliography{docs/content/bib/test_duplicate.bib}'
        self.convert(md)
        self.assertInLogError('repeated bibliograhpy entry: macroTestKey', index=-3)


if __name__ == '__main__':
    unittest.main(verbosity=2)
