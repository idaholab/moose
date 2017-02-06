import os
import unittest
import MooseDocs
from MooseDocs.testing import MarkdownTestCase

class TestCSS(MarkdownTestCase):
    """
    Test that !css command is work.
    """
    def testCSS(self):
        with open(os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'tests', 'css', 'css.md'), 'r') as fid:
            md = fid.read()
        self.assertConvert('test_css.html', md)
