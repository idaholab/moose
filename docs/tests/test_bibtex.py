#!/usr/bin/env python
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestBibtexExtension(MarkdownTestCase):
  """
  Tests that Bibtex is working correctly.
  """

  def testCite(self):
    md = r'\cite{testkey}\n\bibliography{bib/moose.bib}'
    self.assertConvert('test_cite.html', md)

  def testCitet(self):
    md = r'\citet{testkey}\n\bibliography{bib/moose.bib}'
    self.assertConvert('test_citet.html', md)

  def testCitep(self):
    md = r'\citep{testkey}\n\bibliography{bib/moose.bib}'
    self.assertConvert('test_citep.html', md)

if __name__ == '__main__':
  unittest.main(module=__name__, verbosity=2)
