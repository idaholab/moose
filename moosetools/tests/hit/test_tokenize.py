#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest

from moosetools import hit


class TestHitTokenize(unittest.TestCase):
    """
    Test the load function.
    """

    def testToken(self):
        tok = hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"", 6, 1980)
        self.assertEqual(tok.type, hit.TokenType.LEFTBRACKET)
        self.assertEqual(tok.value, b"[")
        self.assertEqual(tok.offset, 6)
        self.assertEqual(tok.line, 1980)

        self.assertEqual(tok, hit.TokenType.LEFTBRACKET)
        self.assertNotEqual(tok, hit.TokenType.RIGHTBRACKET)

        self.assertEqual(tok, hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"", 6, 1980))
        self.assertNotEqual(
            tok, hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"", 10, 1980)
        )

        with self.assertRaises(TypeError) as cm:
            tok == "WRONG"
        self.assertIn(
            "Comparison is only valid with Token and TokenType, <class 'str'> provided",
            str(cm.exception),
        )

    def testTokenize(self):
        text = "[Kernels]\n  [diff]\n    type = Diffusion # DOOSE!\n  []\n[]"
        tokens = hit.tokenize(text, filename="simple_diffusion.i")
        self.assertEqual(len(tokens), 17)
        self.assertEqual(
            tokens[0],
            hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"simple_diffusion.i", 0, 1),
        )
        self.assertEqual(
            tokens[1],
            hit.Token(hit.TokenType.PATH, b"Kernels", b"simple_diffusion.i", 1, 1),
        )
        self.assertEqual(
            tokens[2],
            hit.Token(hit.TokenType.RIGHTBRACKET, b"]", b"simple_diffusion.i", 8, 1),
        )
        self.assertEqual(
            tokens[3],
            hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"simple_diffusion.i", 12, 2),
        )
        self.assertEqual(
            tokens[4],
            hit.Token(hit.TokenType.PATH, b"diff", b"simple_diffusion.i", 13, 2),
        )
        self.assertEqual(
            tokens[5],
            hit.Token(hit.TokenType.RIGHTBRACKET, b"]", b"simple_diffusion.i", 17, 2),
        )
        self.assertEqual(
            tokens[6],
            hit.Token(hit.TokenType.IDENT, b"type", b"simple_diffusion.i", 23, 3),
        )
        self.assertEqual(
            tokens[7],
            hit.Token(hit.TokenType.EQUALS, b"=", b"simple_diffusion.i", 28, 3),
        )
        self.assertEqual(
            tokens[8],
            hit.Token(hit.TokenType.STRING, b"Diffusion", b"simple_diffusion.i", 30, 3),
        )
        self.assertEqual(
            tokens[9],
            hit.Token(
                hit.TokenType.INLINECOMMENT, b"# DOOSE!", b"simple_diffusion.i", 40, 3
            ),
        )
        self.assertEqual(
            tokens[10],
            hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"simple_diffusion.i", 51, 4),
        )
        self.assertEqual(
            tokens[11],
            hit.Token(hit.TokenType.PATH, b"", b"simple_diffusion.i", 52, 4),
        )
        self.assertEqual(
            tokens[12],
            hit.Token(hit.TokenType.RIGHTBRACKET, b"]", b"simple_diffusion.i", 52, 4),
        )
        self.assertEqual(
            tokens[13],
            hit.Token(hit.TokenType.LEFTBRACKET, b"[", b"simple_diffusion.i", 54, 5),
        )
        self.assertEqual(
            tokens[14],
            hit.Token(hit.TokenType.PATH, b"", b"simple_diffusion.i", 55, 5),
        )
        self.assertEqual(
            tokens[15],
            hit.Token(hit.TokenType.RIGHTBRACKET, b"]", b"simple_diffusion.i", 55, 5),
        )
        self.assertEqual(
            tokens[16],
            hit.Token(hit.TokenType.EOF, b"", b"simple_diffusion.i", 56, 5),
        )


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2)
