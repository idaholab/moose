#!/usr/bin/env python
import unittest
import os
import re
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
    repl = 'data-moose-bibfiles="[u\'{}\']"'.format(os.path.abspath(os.path.join(os.getcwd(), '..', '..', 'bib', 'moose.bib')))
    html[0] = re.sub(r'data-moose-bibfiles=\"\[.*?\]\"', repl, html[0])
    return html


  def testCite(self):
    md = r'\cite{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_cite.html', md)

  def testCitet(self):
    md = r'\citet{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_citet.html', md)

  def testCitep(self):
    md = r'\citep{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_citep.html', md)

if __name__ == '__main__':
  unittest.main(module=__name__, verbosity=2)
