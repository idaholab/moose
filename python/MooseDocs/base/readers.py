"""Defines Reader objects that convert raw text into an AST."""
import logging

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import mixins
from MooseDocs.tree import tokens
from lexers import RecursiveLexer

LOG = logging.getLogger(__name__)

class Reader(mixins.ConfigObject, mixins.TranslatorObject, mixins.ComponentObject):
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
        mixins.TranslatorObject.__init__(self)
        common.check_type('lexer', lexer, RecursiveLexer)
        self.__lexer = lexer

    @property
    def lexer(self):
        """
        Return the Lexer object for the reader, this is useful for preforming nested parsing as
        is the case for the markdown parsing done by MooseDocs, see core.py for examples.
        """
        return self.__lexer

    def reinit(self):
        """
        Call the Component reinit() methods.
        """
        for comp in self.components:
            comp.reinit()

    def parse(self, root, content, group=None):
        """
        Perform the parsing of the supplied content into an AST with the provided root node.

        Inputs:
            root[tokens.Token]: The root node for the AST.
            content[unicode:tree.page.PageNodeBase]: The content to parse, either as a unicode
                                                     string or a page node object.
        """
        # Type checking
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('root', root, tokens.Token)
            common.check_type('content', content, unicode)

        # Re-initialize
        config = self.getConfig() #
        self.reinit()

        # Pre-tokenize
        self.translator.executeExtensionFunction('preTokenize', root, config)

        # Tokenize
        self.__lexer.tokenize(root, self.__lexer.grammer(group), content)

        # Post-tokenize
        self.translator.executeExtensionFunction('postTokenize', root, config)

        # Report errors
        for token in anytree.PreOrderIter(root):
            if isinstance(token, tokens.ErrorToken):
                LOG.error(token.report(self.translator.current))

    def add(self, group, component, location='_end'):
        """
        Add a component to Extened the Reader by adding a TokenComponent.

        Inputs:
            group[str]: Name of the lexer group to append.
            component[components.TokenComponent]: The tokenize component to add.
            location[str|int]: The location to insert this component (see Grammer.py)
        """
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type("group", group, str)
            common.check_type("component", component, MooseDocs.base.components.TokenComponent)
            common.check_type("location", location, (str, int))

        # Define the name of the component being added (for sorting within Grammer)
        name = component.__class__.__name__

        # Store and init component, checking self.initialized() allows this object to be used
        # without the Translator which is useful in some cases.
        self.addComponent(component)
        if self.initialized():
            component.init(self.translator)

        # Update the lexer
        self.__lexer.add(group, name, component.RE, component, location)

class MarkdownReader(Reader):
    """
    Reader designed to work with the 'block' and 'inline' structure of markdown to html conversion.

    TODO: Re-investigate removing the 'inline' vs. 'block', it really should be possible if
          the regex's are designed and ordered correctly. Use 'content' or something similar for
          recursion.
    """
    def __init__(self, **kwargs):
        Reader.__init__(self,
                        lexer=RecursiveLexer(MooseDocs.BLOCK, MooseDocs.INLINE),
                        **kwargs)

    def addBlock(self, component, location='_end'):
        """
        Add a component to the 'block' grammer.
        """
        Reader.add(self, MooseDocs.BLOCK, component, location)

    def addInline(self, component, location='_end'):
        """
        Add an inline component to the 'inline' grammer.
        """
        Reader.add(self, MooseDocs.INLINE, component, location)
