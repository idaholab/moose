#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import re
from markdown.blockprocessors import BlockProcessor
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

class AdmonitionExtension(MooseMarkdownExtension):
    """
    Extension for creating admontion (e.g, warning, errors, info, etc.).
    """
    @staticmethod
    def defaultConfig():
        """
        Default configuration options for SQAExtension
        """
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds components to AdmonitionExtension.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        md.parser.blockprocessors.add('moose_admonition',
                                      AdmonitionBlock(markdown_instance=md, **config),
                                      '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """
    Create SQAExtension
    """
    return AdmonitionExtension(*args, **kwargs)

class AdmonitionBlock(MooseMarkdownCommon, BlockProcessor):
    """
    Adds an admonition functionality using syntax similar to other MOOSE syntax.
    """
    RE = re.compile(r'!admonition\s+'
                    r'(?P<command>info|note|important|warning|danger|error)\s*' # commands
                    r'(?P<title>[^\n]*?)'               # optional title (any non newline)
                    r'(?P<settings>\w+=.*?)?'           # optional settings
                    r'\n(?P<message>.*?)(?:\Z|\n{2,})', # message
                    flags=re.DOTALL|re.MULTILINE)

    @staticmethod
    def defaultSettings():
        """Settings for AdmonitionBlock"""
        settings = MooseMarkdownCommon.defaultSettings()
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        BlockProcessor.__init__(self, markdown_instance.parser)
        self.markdown = markdown_instance

    def test(self, parent, block):
        """
        Check that block contains the defined RE.
        """
        return self.RE.search(block)

    def run(self, parent, blocks):
        """
        Create the collapsible region with the listed requirements.
        """
        block = blocks.pop(0)
        match = self.RE.search(block)
        command = match.group('command')
        title = match.group('title').strip()
        message = match.group('message').strip()
        self.createAdmonition(command, message, title=title, parent=parent)
