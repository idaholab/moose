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
from MooseDocs.tree import tokens

Test = tokens.newToken("Test", foo="bar")


class TestTokens(unittest.TestCase):

    def testToken(self):
        token = Test(None)
        self.assertEqual(token.name, "Test")
        self.assertTrue(token["recursive"])
        self.assertEqual(token["foo"], "bar")

        token = Test(None, recursive=False, foo="foo")
        self.assertFalse(token["recursive"])
        self.assertEqual(token["foo"], "foo")

    def testToDict(self):
        token = Test(None)
        self.assertEqual(
            token.toDict(),
            {
                "attributes": {"foo": "bar", "recursive": True},
                "children": [],
                "name": "Test",
            },
        )

    def testCopyToToken(self):
        from MooseDocs.extensions import core

        token = Test(None)
        core.Word(token, content="Word")
        core.Space(token, count=1)
        core.Word(token, content="Word")

        t2 = Test(None)
        token.copyToToken(t2)

        self.assertEqual(t2(0).name, "Word")
        self.assertEqual(t2(0)["content"], "Word")

        self.assertEqual(t2(1).name, "Space")
        self.assertEqual(t2(1)["count"], 1)

        self.assertEqual(t2(2).name, "Word")
        self.assertEqual(t2(2)["content"], "Word")

    def testText(self):
        from MooseDocs.extensions import core

        # 'HeatPipe3Phase Component' tokenizes to Word, Number, Word, Space, Word
        token = Test(None)
        core.Word(token, content="HeatPipe")
        core.Number(token, content="3")
        core.Word(token, content="Phase")
        core.Space(token, count=1)
        core.Word(token, content="Component")
        self.assertEqual(token.text(), "HeatPipe3Phase Component")
        self.assertEqual(token.text("-"), "HeatPipe3Phase-Component")

        # Punctuation still separates words
        token = Test(None)
        core.Word(token, content="Foo")
        core.Punctuation(token, content="-")
        core.Word(token, content="bar")
        self.assertEqual(token.text(), "Foo bar")


if __name__ == "__main__":
    unittest.main(verbosity=2)
