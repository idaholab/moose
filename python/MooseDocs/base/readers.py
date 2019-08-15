#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines Reader objects that convert raw text into an AST."""
import os
import logging

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import mixins
from MooseDocs.tree import tokens, pages
from .lexers import RecursiveLexer

LOG = logging.getLogger(__name__)

class Reader(mixins.ConfigObject, mixins.ComponentObject):
    """
    Base class for reading (parsing) files into AST.

    Inputs:
       lexer[lexers.Lexer]: Object responsible for applying tokenization (see lexers.py).
       **kwargs: key-value pairs passed to configuration settings (see ConfigObject.py).

    Usage:
        In general, it is not necessary to deal directly with the Reader beyond construction.
        The reader should be passed into the Translator, which handles all the necessary calls.
    """
    def __init__(self, lexer, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.ComponentObject.__init__(self)
        common.check_type('lexer', lexer, RecursiveLexer)
        self.__lexer = lexer

    def getRoot(self):
        """
        Create the AST root node.

        This is called by the Translator object.
        """
        return tokens.Token(None)

    def read(self, page):
        """
        Read and return the content of the supplied page.

        This is called by the Translator object.
        """
        if isinstance(page, pages.Source) and page.source and os.path.exists(page.source):
            LOG.debug('READ %s', page.source)
            return common.read(page.source).lstrip('\n')
        elif isinstance(page, pages.Text):
            return page.content

    def tokenize(self, root, content, page, group=None, line=1, report=True):
        """
        Perform the parsing of the supplied content into an AST with the provided root node.

        Inputs:
            root[tokens.Token]: The root node for the AST.
            content[str:tree.page.PageNodeBase]: The content to parse, either as a str
                                                     string or a page node object.
        """
        # Type checking
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('root', root, tokens.Token)
            common.check_type('content', content, str)

        # Tokenize
        self.__lexer.tokenize(root, content, page, self.__lexer.grammar(group), line)

        # Report errors
        if report:
            for token in anytree.PreOrderIter(root):
                if token.name == 'ErrorToken':
                    msg = common.report_error(token['message'],
                                              page.source,
                                              token.info.line if token.info else None,
                                              token.info[0] if token.info else token.text(),
                                              token['traceback'],
                                              u'TOKENIZE ERROR')
                    LOG.error(msg)

    def add(self, group, component, location='_end'):
        """
        Add a component to extend the Reader by adding a TokenComponent.

        This method is called when adding ReaderComonents in the ::extend method.

        Inputs:
            group[str]: Name of the lexer group to append.
            component[components.TokenComponent]: The tokenize component to add.
            location[str|int]: The location to insert this component (see Grammar.py)
        """
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type("group", group, str)
            common.check_type("component", component, MooseDocs.base.components.TokenComponent)
            common.check_type("location", location, (str, int))

        component.setReader(self)
        self.addComponent(component)

        # Update the lexer
        name = component.__class__.__name__
        self.__lexer.add(group, name, component.RE, component, location)

    def preTokenize(self, ast, page, meta):
        """
        Called by Translator prior to tokenization.

        Inputs:
            ast[tokens.Token]: The root node of the token tree.
        """
        pass

    def postTokenize(self, ast, page, meta):
        """
        Called by Translator after tokenization.

        Inputs:
            ast[tokens.Token]: The root node of the token tree.
        """
        pass

class MarkdownReader(Reader):
    """
    Reader designed to work with the 'block' and 'inline' structure of markdown to html conversion.
    """
    EXTENSIONS = ('.md',)

    def __init__(self, **kwargs):
        Reader.__init__(self,
                        lexer=RecursiveLexer(MooseDocs.BLOCK, MooseDocs.INLINE),
                        **kwargs)

    def addBlock(self, component, location='_end'):
        """
        Add a component to the 'block' grammar.
        """
        Reader.add(self, MooseDocs.BLOCK, component, location)

    def addInline(self, component, location='_end'):
        """
        Add an inline component to the 'inline' grammar.
        """
        Reader.add(self, MooseDocs.INLINE, component, location)
