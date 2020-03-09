#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest

from MooseDocs.tree import latex

class TestLatex(unittest.TestCase):
    """
    Tests for latex tree structure.
    """
    def testEscape(self):
        self.assertEqual(latex.escape('&'), '\&')
        self.assertEqual(latex.escape('%'), '\%')
        self.assertEqual(latex.escape('$'), '\$')
        self.assertEqual(latex.escape('#'), '\#')
        self.assertEqual(latex.escape('_'), '\_')
        self.assertEqual(latex.escape('{'), '\{')
        self.assertEqual(latex.escape('}'), '\}')
        self.assertEqual(latex.escape('^'), '{\\textasciicircum}')
        self.assertEqual(latex.escape('~'), '{\\textasciitilde}')
        self.assertEqual(latex.escape('\\'), '{\\textbackslash}')
        self.assertEqual(latex.escape('<'), '{\\textless}')
        self.assertEqual(latex.escape('>'), '{\\textgreater}')

    def testEnclosureBase(self):
        enc = latex.EnclosureBase('foo', None, enclose=('#','@'))
        self.assertEqual(enc['enclose'], ('#','@'))

        enc = latex.EnclosureBase('foo', None, enclose=(';',';'), string='foo')
        self.assertIsInstance(enc(0), latex.String)
        self.assertEqual(enc(0)['content'], 'foo')

    def testBracket(self):
        enc = latex.Bracket()
        self.assertEqual(enc['enclose'], ('[',']'))

    def testBrace(self):
        enc = latex.Brace()
        self.assertEqual(enc['enclose'], ('{','}'))

    def testInlineMath(self):
        enc = latex.InlineMath()
        self.assertEqual(enc['enclose'], ('$','$'))

    def testCommand(self):
        cmd = latex.Command(None, 'foo')
        self.assertEqual(cmd.write(), '\\foo')

        cmd = latex.Command(None, 'foo', string='bar')
        self.assertIsInstance(cmd(0), latex.String)
        self.assertEqual(cmd(0)['content'], 'bar')
        self.assertEqual(cmd.write(), '\\foo{bar}')

        cmd = latex.Command(None, 'foo', start='5', end='6')
        self.assertEqual(cmd.write(), '5\\foo6')

    def testEnvironment(self):
        env = latex.Environment(None, 'foo')
        self.assertEqual(env.write(), '\n\\begin{foo}\n\n\\end{foo}\n')

    def testString(self):
        s = latex.String(None, content='foo')
        self.assertEqual(s['content'], 'foo')


if __name__ == '__main__':
    unittest.main(verbosity=2)
