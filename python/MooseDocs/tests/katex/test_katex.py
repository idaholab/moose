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

class TestKatexExtension(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.katex']

    def testInline(self):
        md = 'This is a $test$ of inline math.'
        html = self.convert(md)
        self.assertIn('document.getElementById', html)
        self.assertIn('<span class="moose-katex-inline" id="moose-katex-equation-', html)

    def testEquation(self):
        md = '\\begin{equation}\nx=y\n\\end{equation}'
        html = self.convert(md)
        self.assertIn('document.getElementById', html)
        self.assertIn('<div class="moose-katex-block">', html)
        self.assertIn('<div class="moose-katex-equation" data-moose-katex-equation-number="1" ' \
                      'id="moose-katex-equation-1">', html)
        self.assertIn('<div class="moose-katex-block-number">', html)

    def testEquationLabel(self):
        md = '\\begin{equation}\nx=y\n\\label{foo}\\end{equation}'
        html = self.convert(md)
        self.assertIn('document.getElementById', html)
        self.assertIn('<div class="moose-katex-block">', html)
        self.assertIn('<div class="moose-katex-equation" data-moose-katex-equation-number="1" ' \
                      'id="moose-katex-equation-foo">', html)
        self.assertIn('<div class="moose-katex-block-number">', html)

    def testStarEquation(self):
        md = '\\begin{equation*}\nx=y\n\\end{equation*}'
        html = self.convert(md)
        self.assertIn('document.getElementById', html)
        self.assertIn('<div class="moose-katex-equation" id="moose-katex-equation-', html)
        self.assertNotIn('<div class="moose-katex-block">', html)
        self.assertNotIn('<div class="moose-katex-equation" data-moose-katex-equation-number="1" ' \
                         'id="moose-katex-equation-1">', html)
        self.assertNotIn('<div class="moose-katex-block-number">', html)

    def testDoubleDollarEquation(self):
        md = '$$\nx=y\n$$'
        html = self.convert(md)
        self.assertIn('document.getElementById', html)
        self.assertIn('<div class="moose-katex-equation" id="moose-katex-equation-', html)
        self.assertNotIn('<div class="moose-katex-block">', html)
        self.assertNotIn('<div class="moose-katex-equation" data-moose-katex-equation-number="1" ' \
                         'id="moose-katex-equation-1">', html)
        self.assertNotIn('<div class="moose-katex-block-number">', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
