#!/usr/bin/env python2
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

    def testEnclosure(self):
        enc = latex.Enclosure(enclose=('#','@'))
        self.assertEqual(enc.enclose, ('#','@'))

        enc = latex.Enclosure(enclose=(';',';'), string=u'foo')
        self.assertIsInstance(enc(0), latex.String)
        self.assertEqual(enc(0).content, 'foo')

    def testBracket(self):
        enc = latex.Bracket()
        self.assertEqual(enc.enclose, ('[',']'))

    def testBrace(self):
        enc = latex.Brace()
        self.assertEqual(enc.enclose, ('{','}'))

    def testInlineMath(self):
        enc = latex.InlineMath()
        self.assertEqual(enc.enclose, ('$','$'))

    def testCommand(self):
        cmd = latex.Command(None, 'foo')
        self.assertEqual(cmd.write(), '\\foo')

        cmd = latex.Command(None, 'foo', string=u'bar')
        self.assertIsInstance(cmd(0), latex.String)
        self.assertEqual(cmd(0).content, 'bar')
        self.assertEqual(cmd.write(), '\\foo{bar}')

        cmd = latex.Command(None, 'foo', start='5', end='6')
        self.assertEqual(cmd.write(), '5\\foo6')

    def testCustomCommand(self):
        cmd = latex.CustomCommand(None, 'foo')
        self.assertEqual(cmd.write(), '\\foo')

        latex.Bracket(cmd, string=u'bar')
        latex.Brace(cmd, string=u'test')

        self.assertEqual(cmd.write(), '\\foo[bar]{test}')

        cmd = latex.CustomCommand(None, 'foo', start='this', end='that')
        self.assertEqual(cmd.write(), 'this\\foothat')

    def testEnvironment(self):
        env = latex.Environment(None, 'foo')
        self.assertEqual(env.write(), '\n\\begin{foo}\n\n\\end{foo}')

    def testString(self):
        s = latex.String(content=u'foo')
        self.assertEqual(s.content, 'foo')


if __name__ == '__main__':
    unittest.main(verbosity=2)
