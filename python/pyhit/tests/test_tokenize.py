#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import pyhit

class TestHitTokenize(unittest.TestCase):
    """
    Test the load function.
    """
    def testToken(self):
        tok = pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 6, 1980)
        self.assertEqual(tok.type, pyhit.TokenType.LEFTBRACKET)
        self.assertEqual(tok.value, b'[')
        self.assertEqual(tok.offset, 6)
        self.assertEqual(tok.line, 1980)

        self.assertEqual(tok, pyhit.TokenType.LEFTBRACKET)
        self.assertNotEqual(tok, pyhit.TokenType.RIGHTBRACKET)

        self.assertEqual(tok, pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 6, 1980))
        self.assertNotEqual(tok, pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 10, 1980))

        with self.assertRaises(TypeError) as cm:
            tok == 'WRONG'
        self.assertIn("Comparison is only valid with Token and TokenType, <class 'str'> provided", str(cm.exception))

    def testTokenize(self):
        text = '[Kernels]\n  [diff]\n    type = Diffusion # DOOSE!\n  []\n[]'
        tokens = pyhit.tokenize(text, filename='simple_diffusion.i')
        self.assertEqual(len(tokens), 17)
        self.assertEqual(tokens[0], pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 0, 1))
        self.assertEqual(tokens[1], pyhit.Token(pyhit.TokenType.PATH, b'Kernels', 1, 1))
        self.assertEqual(tokens[2], pyhit.Token(pyhit.TokenType.RIGHTBRACKET, b']', 8, 1))
        self.assertEqual(tokens[3], pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 12, 2))
        self.assertEqual(tokens[4], pyhit.Token(pyhit.TokenType.PATH, b'diff', 13, 2))
        self.assertEqual(tokens[5], pyhit.Token(pyhit.TokenType.RIGHTBRACKET, b']', 17, 2))
        self.assertEqual(tokens[6], pyhit.Token(pyhit.TokenType.IDENT, b'type', 23, 3))
        self.assertEqual(tokens[7], pyhit.Token(pyhit.TokenType.EQUALS, b'=', 28, 3))
        self.assertEqual(tokens[8], pyhit.Token(pyhit.TokenType.STRING, b'Diffusion', 30, 3))
        self.assertEqual(tokens[9], pyhit.Token(pyhit.TokenType.INLINECOMMENT, b'# DOOSE!', 40, 3))
        self.assertEqual(tokens[10], pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 51, 4))
        self.assertEqual(tokens[11], pyhit.Token(pyhit.TokenType.PATH, b'', 52, 4))
        self.assertEqual(tokens[12], pyhit.Token(pyhit.TokenType.RIGHTBRACKET, b']', 52, 4))
        self.assertEqual(tokens[13], pyhit.Token(pyhit.TokenType.LEFTBRACKET, b'[', 54, 5))
        self.assertEqual(tokens[14], pyhit.Token(pyhit.TokenType.PATH,b'',55, 5))
        self.assertEqual(tokens[15], pyhit.Token(pyhit.TokenType.RIGHTBRACKET, b']', 55, 5))
        self.assertEqual(tokens[16], pyhit.Token(pyhit.TokenType.EOF, b'', 56, 5))

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
