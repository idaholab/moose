#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import collections
from MooseDocs.common import load_extensions
from MooseDocs import base
from MooseDocs.tree import tokens, pages, html, latex
from MooseDocs.extensions import command

class MooseDocsTestCase(unittest.TestCase):
    """
    A class to aid in creating unit tests for MOOSEDocs.
    """
    EXTENSIONS = []
    READER = base.MarkdownReader
    RENDERER = base.HTMLRenderer
    EXECUTIONER = base.Serial

    def __init__(self, *args, **kwargs):
        super(MooseDocsTestCase, self).__init__(*args, **kwargs)
        # ast = Abstract Syntax Tree
        self.__ast = None
        self.__result = None
        self.__translator = None
        self.__text = pages.Text(mutable=True)

    def __setup(self, reader=None, renderer=None, extensions=None, executioner=None):
        """Helper method for setting up MOOSEDocs objects. This is called automatically."""

        command.CommandExtension.EXTENSION_COMMANDS.clear()

        content = [self.__text] + (self.setupContent() or [])
        reader = self.READER() if reader is None else reader
        renderer = self.RENDERER() if renderer is None else renderer
        extensions = extensions or self.EXTENSIONS
        executioner = executioner or self.EXECUTIONER()

        config = dict()
        for ext in extensions:
            local = self.setupExtension(ext)
            if local:
                config[ext.__name__] = local

        ext = load_extensions(extensions, config)
        self.__translator = base.Translator(content, reader, renderer, ext, executioner)
        self.__translator.init()
        self.__translator.execute() # This is required to setup the meta data

    def setupExtension(self, ext):
        """Virtual method for child objects to update extension configuration."""
        pass

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        return []

    def tokenize(self, text, *args, **kwargs):
        """Helper for tokenization"""
        if args or kwargs or (self.__translator is None):
            self.__setup(*args, **kwargs)

        self.__translator.executioner.read(self.__text) # runs pre- and post-read methods
        self.__ast = self.__translator.executioner.tokenize(self.__text, text)
        return self.__ast

    def render(self, ast, *args, **kwargs):
        """Helper for rendering AST"""
        if args or kwargs or (self.__translator is None):
            self.__setup(*args, **kwargs)
            self.tokenize('')
        self.__result = self.__translator.executioner.render(self.__text, ast)
        return self.__result

    def execute(self, text, *args, **kwargs):
        """Helper for tokenization and renderering"""
        self.tokenize(text, *args, **kwargs)
        self.render(self.__ast)
        return self.__ast, self.__result

    def assertToken(self, token, tname, string=None, size=None, **kwargs):
        """Helper for checking type and attributes of a token"""
        self.assertEqual(token.name, tname)
        self.assertAttributes(token, **kwargs)

        if size is not None:
            self.assertSize(token, size)
        if string is not None:
            self.assertSize(token, 1)
            self.assertToken(token(0), 'String', content=string)

    def assertHTMLTag(self, tag, tname, string=None, size=None, **kwargs):
        """Helper for checking HTML tree nodes"""
        self.assertIsInstance(tag, html.Tag)
        self.assertEqual(tag.name, tname)
        self.assertAttributes(tag, **kwargs)

        if size is not None:
            self.assertSize(tag, size)
        if string is not None:
            self.assertSize(tag, 1)
            self.assertHTMLString(tag(0), string)

    def assertHTMLString(self, node, content, **kwargs):
        self.assertIsInstance(node, html.String)
        self.assertEqual(node.get('content'), content)
        self.assertAttributes(node, **kwargs)

    def assertLatex(self, node, tname, name, string=None, size=None, nargs=None, **kwargs):
        self.assertEqual(node.__class__.__name__, tname)
        self.assertEqual(node.name, name)
        self.assertAttributes(node, **kwargs)

        if size is not None:
            self.assertSize(node, size)
        if string is not None:
            self.assertSize(node, 1)
            self.assertLatexString(node(0), string)
        if nargs is not None:
            self.assertEqual(len(node['args']), nargs)

    def assertLatexCommand(self, *args, **kwargs):
        self.assertLatex(args[0], 'Command', *args[1:], **kwargs)

    def assertLatexEnvironment(self, *args, **kwargs):
        self.assertLatex(args[0], 'Environment', *args[1:], **kwargs)

    def assertLatexString(self, node, content, **kwargs):
        self.assertIsInstance(node, latex.String)
        self.assertEqual(node.get('content'), content)
        self.assertAttributes(node, **kwargs)

    def assertLatexArg(self, node, index, tname, string=None, size=None, **kwargs):
        arg = node['args'][index]
        self.assertLatex(arg, tname, tname, string, size, **kwargs)

    def assertAttributes(self, node, **kwargs):
        for key, value in kwargs.items():
            key = key.rstrip('_')
            self.assertEqual(node[key], value)

    def assertSize(self, node, num):
        self.assertEqual(len(node), num)
