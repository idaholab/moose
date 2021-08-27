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

import moosetree

import MooseDocs
from .. import common
from ..common import mixins
from ..tree import tokens, pages
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
    __TRANSLATOR_METHODS__ = ['init',
                              'initPage',
                              'read',
                              'tokenize',
                              'preExecute', 'postExecute',
                              'preRead', 'postRead',
                              'preTokenize', 'postTokenize']


    def __init__(self, lexer, **kwargs):
        mixins.ConfigObject.__init__(self, 'reader', **kwargs)
        mixins.ComponentObject.__init__(self)
        self.__lexer = lexer

    def getRoot(self):
        """
        Create the AST root node.

        This is called by the Translator object.
        """
        return tokens.Token(None)

    def add(self, group, component, location='_end'):
        """
        Add a component to extend the Reader by adding a ReaderComponent.

        This method is called when adding ReaderComonents in the ::extend method.

        Inputs:
            group[str]: Name of the lexer group to append.
            component[components.ReaderComponent]: The tokenize component to add.
            location[str|int]: The location to insert this component (see Grammar.py)
        """
        component.setReader(self)
        self.addComponent(component)

        # Update the lexer
        name = component.__class__.__name__
        self.__lexer.add(group, name, component.RE, component, location)


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
        # Tokenize
        self.__lexer.tokenize(root, content, page, self.__lexer.grammar(group), line)

        # Report errors
        if report:
            for token in moosetree.iterate(root):
                if token.name == 'ErrorToken':
                    msg = common.report_error(token['message'],
                                              page.source,
                                              token.info.line if token.info else None,
                                              token.info[0] if token.info else token.text(),
                                              token['traceback'],
                                              'TOKENIZE ERROR')
                    LOG.error(msg)

    def init(self):
        """
        Called after Translator is set, prior to initializing pages.
        """
        pass

    def initPage(self, page):
        """
        Called for each Page object during initialization.
        """
        pass

    def preExecute(self):
        """
        Called by Translator prior to beginning conversion.
        """
        pass

    def postExecute(self):
        """
        Called by Translator after all conversion is complete.
        """
        pass

    def preRead(self, page):
        """
        Called after to reading the file.

        Input:
             page[pages.Source]: The source object representing the content
        """
        pass

    def postRead(self, page, content):
        """
        Called after to reading the file.

        Input:
             page[pages.Source]: The source object representing the content
             content[str]: A copy of the content read from the page
        """
        pass

    def preTokenize(self, page, content, ast):
        """
        Called by Translator prior to tokenization.

        Inputs:
            page[pages.Source]: The source object representing the content
            content[str]: A copy of the content read from the page
            ast[tokens.Token]: The root node of the token tree
        """
        pass

    def postTokenize(self, page, ast):
        """
        Called by Translator after tokenization.

        Inputs:
            page[pages.Source]: The source object representing the content
            ast[tokens.Token]: The root node of the token tree
        """
        pass

class MarkdownReader(Reader):
    """
    Reader designed to work with the 'block' and 'inline' structure of markdown to html conversion.
    """
    BLOCK = 'block'
    INLINE = 'inline'
    EXTENSIONS = ('.md',)

    def __init__(self, **kwargs):
        Reader.__init__(self,
                        lexer=RecursiveLexer(MarkdownReader.BLOCK, MarkdownReader.INLINE),
                        **kwargs)

    def addBlock(self, component, location='_end'):
        """
        Add a component to the 'block' grammar.
        """
        Reader.add(self, MarkdownReader.BLOCK, component, location)

    def addInline(self, component, location='_end'):
        """
        Add an inline component to the 'inline' grammar.
        """
        Reader.add(self, MarkdownReader.INLINE, component, location)
