#!/usr/bin/env python
import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestBibtexExtension(MarkdownTestCase):
    """
    Tests that Bibtex is working correctly.
    """
    def readGold(self, name):
        """
        The parsed markdown contains a path to ".../moose/docs/bib/moose.bib".
        This puts in the correct path after given the HTML that comes before
        and after the string "/Users/<username>/<intermediate_directories>/moose".
        """
        html = super(TestBibtexExtension, self).readGold(name)
        html[0] = html[0].replace('<<CWD>>', os.path.abspath(os.path.join(MooseDocs.MOOSE_DIR, 'docs')))
        return html

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
