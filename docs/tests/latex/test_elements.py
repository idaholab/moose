#!/usr/bin/env python
import os
import unittest
import tempfile
import MooseDocs
from MooseDocs import html2latex
from MooseDocs import testing

class TestLatexElements(testing.TestLatexBase):
    """
    Test that basic html to latex conversion working.
    """
    def test_h1(self):
        md = '# Heading 1'
        gold = u'\n\\section{Heading 1\\label{sec:heading-1}}'
        self.assertLaTeX(md, gold)

if __name__ == '__main__':
    unittest.main(verbosity=2)
