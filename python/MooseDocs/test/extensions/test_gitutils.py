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
from unittest import mock
import logging
import datetime as dt
from MooseDocs import common, base
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, gitutils
logging.basicConfig()

class TestGitUtilsCommit(MooseDocsTestCase):
    EXTENSIONS = [core, command, gitutils]

    def testBlock(self):
        ast = self.tokenize('!git commit')
        self.assertToken(ast(0), 'Word')
        self.assertEqual(len(ast(0)['content']), 40)

    def testInline(self):
        ast = self.tokenize('[!git!commit]')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Word')
        self.assertEqual(len(ast(0,0)['content']), 40)

    def testContentException(self):
        ast = self.tokenize('!git commit\nContent')
        self.assertToken(ast(0), "ErrorToken")
        self.assertIn("Content is not supported for the 'git commit' command.", ast(0)['message'])

        with mock.patch('mooseutils.git_is_repo') as git_is_repo:
            git_is_repo.return_value = False
            ast = self.tokenize('!git commit')
        self.assertToken(ast(0), "ErrorToken")
        self.assertIn("The current working directory is not a git repository.", ast(0)['message'])

class TestGitUtilsCommit(MooseDocsTestCase):
    EXTENSIONS = [core, command, gitutils]

    def testInline(self):
        ast = self.tokenize('[!git!submodule-hash](petsc)')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Word')
        self.assertEqual(len(ast(0,0)['content']), 40)

    def testInlineUrl(self):
        ast = self.tokenize('[!git!submodule-hash url=https://foo.com](petsc)')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link')
        self.assertEqual(len(ast(0,0,0)['content']), 40)

    def testContentException(self):
        ast = self.tokenize('[!git!submodule-hash](wrong)')
        self.assertToken(ast(0,0), "ErrorToken")
        self.assertIn("The submodule 'wrong' was not located", ast(0,0)['message'])

    def testJoin(self):
        ast = self.tokenize('[!git!submodule-hash url=https://github.com/petsc/petsc/commit/](petsc)')
        link = ast(0,0)
        self.assertIn('/commit/', link['url'])

        ast = self.tokenize('[!git!submodule-hash url=https://github.com/petsc/petsc/commit](petsc)')
        link = ast(0,0)
        self.assertIn('/commit/', link['url'])

        ast = self.tokenize('[!git!submodule-hash url=https://github.com/petsc/petsc/commit///](petsc)')
        link = ast(0,0)
        self.assertIn('/commit/', link['url'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
