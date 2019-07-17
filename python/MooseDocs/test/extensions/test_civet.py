#!/usr/bin/env python3
import unittest
import logging
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, civet
from MooseDocs import base
logging.basicConfig()

class TestInlineCivet(MooseDocsTestCase):
    EXTENSIONS = [core, command, civet]
    TEXT = u"[!civet!results owner=idaholab repo=moose](Results)"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'Link', size=1)
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content=u'Results')

        url = ast(0)(0)['url']
        self.assertIn('https://civet.inl.gov/sha_events/idaholab/moose/', url)
        sha = url[48:]
        self.assertEqual(len(sha), 40)

if __name__ == '__main__':
    unittest.main(verbosity=2)
