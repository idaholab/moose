"""Module for common MooseDocs unittest related tasks."""
import unittest
import inspect

import MooseDocs
from MooseDocs import base, common, tree
from mooseutils import text_diff

class MooseDocsTestCase(unittest.TestCase):
    """
    TestCase object for converting markdown to AST, HTML, and LaTeX.
    """
    EXTENSIONS = ['MooseDocs.extensions.core']
    EXTENSIONS_CONFIG = dict()
    READER = base.MarkdownReader
    RENDERER = base.HTMLRenderer
    CONFIG = dict()

    def setUpContent(self): #pylint: disable=no-self-use
        """
        Return the page tree for the translator.
        """
        return tree.page.PageNodeBase(None)

    def setUp(self):
        """
        Create the Translator instance.
        """
        self._reader = self.READER()
        self._renderer = self.RENDERER()
        extensions = common.load_extensions(self.EXTENSIONS, self.EXTENSIONS_CONFIG)
        content = self.setUpContent()
        self._translator = base.Translator(content, self._reader, self._renderer, extensions,
                                           **self.CONFIG)
        self._translator.init()

    def assertString(self, content, gold):
        """
        Assert the rendered html string.

        Inputs:
            ast: HTML tree.
        """
        self.assertEqual(content, gold, text_diff(content, gold))

    def ast(self, content):
        """
        Create AST from Reader object.
        """
        ast = tree.tokens.Token(None)
        self._translator.reader.parse(ast, content, group=MooseDocs.BLOCK)
        return ast

    def render(self, content):
        """
        Convert text into rendered content.
        """
        return self._translator.renderer.render(self.ast(content))

def get_parent_objects(module, cls):
    """
    Tool for locating all objects that derive from a certain base class.
    """
    func = lambda obj: inspect.isclass(obj) and issubclass(obj, cls)
    return inspect.getmembers(module, predicate=func)
