#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import logging

import markdown

import mooseutils
from MooseDocs.commands.MarkdownNode import MarkdownNode

LOG = logging.getLogger(__name__)

class MooseMarkdown(markdown.Markdown):
    """
    A custom Markdown object for handling raw text, markdown files, or MarkdownNode objects.

    The key to this class is allowing the Markdown object to work with MarkdownNode objects such
    that the extension objects, namely MooseTemplate, could have access to the node object to allow
    for searching the tree for other pages. This should allow for cross page figure, equation, and
    table links to be created.
    """
    CODE_BLOCK_COUNT = 0 # counter for code block copy buttons

    def __init__(self, extensions=None, extension_configs=None):

        # Initialize member variables
        self.current = None # member for holding the current MarkdownNode object
        if extensions is None:
            extensions = []
        if extension_configs is None:
            extension_configs = dict()

        super(MooseMarkdown, self).__init__(extensions=extensions,
                                            extension_configs=extension_configs)

    def requireExtension(self, required):
        """
        Raise an exception of the supplied extension type is not registered.
        """
        if not self.getExtension(required):
            raise mooseutils.MooseException("The {} extension is required." \
                                            .format(required.__name__))

    def getExtension(self, etype):
        """
        Return an extension instance.

        Args:
            etype[type]: The type of the extension to return.
        """
        out = None
        for ext in self.registeredExtensions:
            if isinstance(ext, etype):
                out = ext
                break
        return out

    def convert(self, md):
        """
        Convert the raw text, markdown file, or node to html.

        Args:
            md[str]: A markdown file, markdown content, or MarkdownNode
        """
        MooseMarkdown.CODE_BLOCK_COUNT = 0
        self.current = None

        if isinstance(md, MarkdownNode):
            self.current = md
        else:
            self.current = MarkdownNode(md, name='')

        return super(MooseMarkdown, self).convert(self.current.content)
