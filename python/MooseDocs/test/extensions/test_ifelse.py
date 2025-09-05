#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import unittest
import logging
from MooseDocs.test import MooseDocsTestCase, requiresMooseExecutable
from MooseDocs.extensions import (
    core,
    command,
    table,
    floats,
    materialicon,
    autolink,
    heading,
    appsyntax,
    ifelse,
    modal,
    alert,
)

logging.basicConfig()


def mockFunction(ext, value):
    return value


class TestHasMooseApp(MooseDocsTestCase):
    EXTENSIONS = [
        core,
        command,
        table,
        floats,
        materialicon,
        autolink,
        heading,
        appsyntax,
        ifelse,
        modal,
        alert,
    ]

    def setupExtension(self, ext):
        sys.path.append(os.path.dirname(__file__))
        if ext == ifelse:
            return dict(active=True, modules=["test_ifelse"])
        if ext is appsyntax:
            return dict(executable=os.path.join(os.getenv("MOOSE_DIR"), "test"))

    @requiresMooseExecutable()
    def testIf(self):
        ast = self.tokenize("!if function=hasMooseApp('MooseTestApp')\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")

        ast = self.tokenize("!if function=hasMooseApp('WrongApp')\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")


class TestCapabilities(MooseDocsTestCase):
    EXTENSIONS = [
        core,
        command,
        table,
        floats,
        materialicon,
        autolink,
        heading,
        appsyntax,
        ifelse,
        modal,
        alert,
    ]

    def setupExtension(self, ext):
        sys.path.append(os.path.dirname(__file__))
        if ext == ifelse:
            return dict(active=True, modules=["test_ifelse"])
        if ext is appsyntax:
            return dict(executable=os.path.join(os.getenv("MOOSE_DIR"), "test"))

    @requiresMooseExecutable()
    def testHasCapability(self):
        ast = self.tokenize("!if function=hasCapability('petsc')\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")

        ast = self.tokenize("!if function=hasCapability('missing')\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")


class TestStatements(MooseDocsTestCase):
    EXTENSIONS = [core, ifelse, command]

    def setupExtension(self, ext):
        sys.path.append(os.path.dirname(__file__))
        if ext == ifelse:
            return dict(active=True, modules=["test_ifelse"])

    def testIf(self):
        # True
        ast = self.tokenize("!if function=mockFunction(True)\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")

        # False
        ast = self.tokenize("!if function=mockFunction(False)\ncontent")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")

    def testIfElse(self):
        # True
        ast = self.tokenize("!if function=mockFunction(True)\ncontent\n\n!else\nelse")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=2)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")
        self.assertToken(ast(0, 1), "Condition", size=0, command="else")

        # False
        ast = self.tokenize("!if function=mockFunction(False)\ncontent\n\n!else\nelse")
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=2)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(ast(0, 1), "Condition", size=1, command="else")
        self.assertToken(ast(0, 1, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 1, 0, 0), "Word", size=0, content="else")

    def testIfElif(self):
        # True/True
        ast = self.tokenize(
            "!if function=mockFunction(True)\ncontent\n\n!elif function=mockFunction(True)\nelif"
        )
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=2)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")
        self.assertToken(ast(0, 1), "Condition", size=0, command="elif")

        # False/True
        ast = self.tokenize(
            "!if function=mockFunction(False)\ncontent\n\n!elif function=mockFunction(True)\nelif"
        )
        self.assertToken(ast(0), "Statement", size=2)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(ast(0, 1), "Condition", size=1, command="elif")
        self.assertToken(ast(0, 1, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 1, 0, 0), "Word", size=0, content="elif")

        # False/False
        ast = self.tokenize(
            "!if function=mockFunction(False)\ncontent\n\n!elif function=mockFunction(False)\nelif"
        )
        self.assertToken(ast(0), "Statement", size=2)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(ast(0, 1), "Condition", size=0, command="elif")

    def testIfElifElse(self):
        # True/True
        ast = self.tokenize(
            "!if function=mockFunction(True)\ncontent\n\n!elif function=mockFunction(True)\nelif\n\n!else\nelse"
        )
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), "Statement", size=3)
        self.assertToken(ast(0, 0), "Condition", size=1, command="if")
        self.assertToken(ast(0, 0, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 0, 0, 0), "Word", size=0, content="content")
        self.assertToken(ast(0, 1), "Condition", size=0, command="elif")
        self.assertToken(ast(0, 2), "Condition", size=0, command="else")

        # False/True
        ast = self.tokenize(
            "!if function=mockFunction(False)\ncontent\n\n!elif function=mockFunction(True)\nelif\n\n!else\nelse"
        )
        self.assertToken(ast(0), "Statement", size=3)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(ast(0, 1), "Condition", size=1, command="elif")
        self.assertToken(ast(0, 1, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 1, 0, 0), "Word", size=0, content="elif")
        self.assertToken(ast(0, 2), "Condition", size=0, command="else")

        # False/False
        ast = self.tokenize(
            "!if function=mockFunction(False)\ncontent\n\n!elif function=mockFunction(False)\nelif\n\n!else\nelse"
        )
        self.assertToken(ast(0), "Statement", size=3)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(ast(0, 1), "Condition", size=0, command="elif")
        self.assertToken(ast(0, 2), "Condition", size=1, command="else")
        self.assertToken(ast(0, 2, 0), "Paragraph", size=1)
        self.assertToken(ast(0, 2, 0, 0), "Word", size=0, content="else")

    def testIfCommandBaseErrors_createTokenHelper(self):
        ast = self.tokenize("!if")
        self.assertToken(
            ast(0), "ErrorToken", message="The 'function' setting is required."
        )

        ast = self.tokenize("!elif function=foo")
        self.assertToken(
            ast(0),
            "ErrorToken",
            message="The 'Condition' being created is out of place, it must be in sequence with an an 'if' and 'elif' condition(s).",
        )

    def testIfCommandBaseErrors_evaluateFunction(self):
        ast = self.tokenize("!if function=invalidFunction(1980\ncontent")
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(
            ast(1),
            "ErrorToken",
            message="Invalid expression for 'function' setting: invalidFunction(1980",
        )

        ast = self.tokenize("!if function=mockFunction(1980)\ncontent")
        self.assertToken(ast(0), "Statement", size=1)
        self.assertToken(ast(0, 0), "Condition", size=0, command="if")
        self.assertToken(
            ast(1),
            "ErrorToken",
            message="The return value from the function 'mockFunction' must be a 'bool' type, but '<class 'int'>' returned.",
        )

    def testElseCommandErrors(self):
        ast = self.tokenize("!else")
        self.assertToken(
            ast(0),
            "ErrorToken",
            message="The 'else' command must follow an 'if' or 'elif' condition.",
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
