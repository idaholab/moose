#!/usr/bin/env python
import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestCSS(MarkdownTestCase):
    """
    Test that !css command is work.
    """
    def testCSS(self):
        self.assertConvertFile('test_css.html', os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'tests', 'css', 'css.md'))

    def testCSSList(self):
        self.assertConvertFile('test_css_list.html', os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'tests', 'css', 'css_list.md'))


if __name__ == '__main__':
    unittest.main(verbosity=2)
