#!/usr/bin/env python
import unittest
import os
import re
from MooseDocs.testing import MarkdownTestCase

class TestBibtexExtension(MarkdownTestCase):
  """
  Tests that Bibtex is working correctly.
  """
  def writeGoldFile(self, file_name, html_array):
    """
    The parsed markdown contains a path to ".../moose/bib/moose.bib".
    This puts in the correct path after given the HTML that comes before
    and after the string "/Users/<username>/<intermediate_directories>/moose",
    and writes the appropriate gold file to compare the generated HTML.
    """
    RE = re.compile(r'.*?/moose')
    dir_match = RE.search(os.getcwd())
    gold_name = os.path.join('gold',file_name)
    with open(gold_name,'w') as goldFile:
      html = html_array[0] + dir_match.group(0) + html_array[1]
      goldFile.write(html)

  def testCite(self):
    html_array = ['<p><a href="#testkey" data-moose-cite="\cite{testkey}">Smith and Doe (1980)</a>\\n<ol class="moose-bibliography" data-moose-bibfiles="[u\'',
             '/docs/bib/moose.bib\']">\n' \
             '<li name="testkey">Jane Smith and John Doe.\n' \
             'A test citation without special characters for easy testing.\n' \
             '<em>A Prestigous Journal</em>, 1980.</li>\n' \
             '</ol>\n' \
             '</p>\n']
    self.writeGoldFile('test_cite.html',html_array)
    md = r'\cite{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_cite.html', md)

  def testCitet(self):
    html_array = ['<p><a href="#testkey" data-moose-cite="\citet{testkey}">Smith and Doe (1980)</a>\\n<ol class="moose-bibliography" data-moose-bibfiles="[u\'',
             '/docs/bib/moose.bib\']">\n' \
             '<li name="testkey">Jane Smith and John Doe.\n' \
             'A test citation without special characters for easy testing.\n' \
             '<em>A Prestigous Journal</em>, 1980.</li>\n' \
             '</ol>\n' \
             '</p>\n']
    self.writeGoldFile('test_citet.html',html_array)
    md = r'\citet{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_citet.html', md)

  def testCitep(self):
    html_array = ['<p>(<a href="#testkey" data-moose-cite="\citep{testkey}">Smith and Doe, 1980</a>)\\n<ol class="moose-bibliography" data-moose-bibfiles="[u\'',
             '/docs/bib/moose.bib\']">\n' \
             '<li name="testkey">Jane Smith and John Doe.\n' \
             'A test citation without special characters for easy testing.\n' \
             '<em>A Prestigous Journal</em>, 1980.</li>\n' \
             '</ol>\n' \
             '</p>\n']
    self.writeGoldFile('test_citep.html',html_array)
    md = r'\citep{testkey}\n\bibliography{docs/bib/moose.bib}'
    self.assertConvert('test_citep.html', md)

if __name__ == '__main__':
  unittest.main(module=__name__, verbosity=2)
