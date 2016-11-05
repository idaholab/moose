#!/usr/bin/env python
import unittest
from MooseDocs.testing import MarkdownTestCase

class TestMarkdownExtensions(MarkdownTestCase):
  """
  Tests that the 'moosedocs.yml' configuration file sets up the markdown conversion, including
  that the basic (i.e., non-MOOSE) extensions are working.
  """

  def testConvert(self):
    md = "Testing"
    html = self.parser.convert(md)
    self.assertEqual(html, '<p>Testing</p>')

  def testToc(self):
    md = "[TOC]\n# Section One\n\n#Section Two"
    self.assertConvert('test_toc.html', md)

  def testAbbreviations(self):
    md = "The HTML specification\n" \
         "is maintained by the W3C.\n\n" \
         "*[HTML]: Hyper Text Markup Language\n" \
         "*[W3C]:  World Wide Web Consortium"
    self.assertConvert('test_abbreviations.html', md)

  def testAttributeLists(self):
    md = "This is a paragraph.\n" \
         "{: #an_id .a_class }\n\n" \
         "A setext style header {: #setext}\n" \
         "=================================\n\n" \
         "### A hash style header ### {: #hash }\n\n" \
         '[link](http://example.com){: class="foo bar" title="Some title!" }'

    self.assertConvert('test_attributelists.html', md)

  def testDefinitionLists(self):
    md = "Apple\n" \
         ":   Pomaceous fruit of plants of the genus Malus in\n" \
         "    the family Rosaceae.\n\n" \
         "Orange\n" \
         ":   The fruit of an evergreen tree of the genus Citrus."
    self.assertConvert('test_definitionlists.html', md)

  def testFencedCodeBlocks(self):
    md = "~~~~{.python}\n" \
         "# python code\n" \
         "~~~~\n\n" \
         "~~~~.html\n" \
         "<p>HTML Document</p>\n" \
         "~~~~"
    self.assertConvert('test_fencedcodeblocks.html', md)

  @unittest.skip("^ in markdown causes trouble")
  def testFootnotes(self):
    md = r"Footnotes[^1] have a label[^@#$%] and the footnote's content.\n\n" \
         r"[^1]: This is a footnote content.\n" \
         r'[^@#$%]: A footnote on the label: "@#$%".'
    self.assertConvert('test_footnotes.html', md)

  def testTables(self):
    md = "First Header  | Second Header\n" \
         "------------- | -------------\n" \
         "Content Cell  | Content Cell\n" \
         "Content Cell  | Content Cell"
    self.assertConvert('test_tables.html', md)

  def testSmartStrong(self):
    md = '__this__works__too__.'
    html = self.parser.convert(md)
    self.assertEqual(html, u'<p><strong>this__works__too</strong>.</p>')

  def testAdmonition(self):
    md = "!!! note\n" \
         "    You should note that the title will be automatically capitalized."
    self.assertConvert('test_admonition.html', md)

  def testSmarty(self):
    html = self.parser.convert("'foo'")
    self.assertEqual(u'<p>&lsquo;foo&rsquo;</p>', html)

    html = self.parser.convert('"foo"')
    self.assertEqual(u'<p>&ldquo;foo&rdquo;</p>', html)

    html = self.parser.convert('...')
    self.assertEqual(u'<p>&hellip;</p>', html)

    html = self.parser.convert('--')
    self.assertEqual(u'<p>&ndash;</p>', html)


if __name__ == '__main__':
  unittest.main(module=__name__, verbosity=2)
