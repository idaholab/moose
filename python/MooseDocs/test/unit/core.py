#!/usr/bin/env python2
import unittest
import logging

from MooseDocs.common import load_extensions
from MooseDocs import base
from MooseDocs.tree import tokens, pages
from MooseDocs.extensions import core

logging.basicConfig()



class MooseDocsTestCase(unittest.TestCase):
    EXTENSIONS = []
    READER = base.MarkdownReader()
    RENDERER = base.HTMLRenderer()
    EXECUTIONER = None

    def __init__(self, *args, **kwargs):
        super(unittest.TestCase, self).__init__(*args, **kwargs)

        self.__translator = None

    def setup(self,content=[], reader=READER, renderer=RENDERER, extensions=EXTENSION,
                 executioner=EXECUTIONER):

        ext = load_extensions(EXTENSIONS)

        self.__translator = base.Translator(content, reader, renderer, extensions=ext, EXECUTIONER)
        self.__translator.init()

    def tokenize(self, text):
        page = pages.Page('TestString', source='string')

        return self.__translator.executioner.tokenize(ast)

    def render(self, ast):




class TestCore(MooseDocsTestCase):
    EXTENSIONS = [core]
    def testCodeBlock(self):

        text = u"```\nint x = 0;\n```"

        """
        page = pages.Page('foo', source='bar')

        ext = load_extensions(['MooseDocs.extensions.core'])
        translator = base.Translator([],
                                     base.MarkdownReader(),
                                     base.MaterializeRenderer(),
                                     extensions=ext)
        translator.init()

        ast = tokens.Token('', None)
        translator.reader.tokenize(ast, text, page)
        """
        print ast



if __name__ == '__main__':
    unittest.main(verbosity=2)
