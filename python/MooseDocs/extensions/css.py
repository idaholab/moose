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
import re

from markdown.preprocessors import Preprocessor
from markdown.blockprocessors import BlockProcessor
from markdown.util import etree

from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

class CSSExtension(MooseMarkdownExtension):
    """
    Adds CSS support via the !css command to MooseDocs markdown.
    """

    @staticmethod
    def defaultConfig():
        """Default configure options for CSSExtension"""
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds specialized css support for MOOSE flavored markdown (i.e., !css markdown syntax)
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose_css_list',
                             CSSPreprocessor(markdown_instance=md, **config), '_end')
        md.parser.blockprocessors.add('moose_css',
                                      CSSBlockProcessor(md.parser, **config), '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create CSSExtension"""
    return CSSExtension(*args, **kwargs)


class CSSBlockProcessor(BlockProcessor, MooseMarkdownCommon):
    """
    Markdown extension for applying CSS styles to paragraph
    Markdown syntax is:
     !css <options>
     Paragraph text here

    Where <options> are key=value pairs.
    """
    RE = re.compile(r'^!css(.*)$', flags=re.MULTILINE)

    # If there are multiple css blocks on the same page then
    # they need to have different ids
    MATCHES_FOUND = 0

    def __init__(self, parser, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        BlockProcessor.__init__(self, parser)

    def test(self, parent, block):
        """
        Test to see if we should process this block of markdown.
        Inherited from BlockProcessor.
        """
        return self.RE.search(block)

    def run(self, parent, blocks):
        """
        Called when it is determined that we can process this block.
        This will convert the markdown into HTML
        """
        sibling = self.lastChild(parent)
        block = blocks.pop(0)
        m = self.RE.search(block)

        if m:
            # Parse out the options on the css line
            settings = self.getSettings(m.group(1))
            block = block[m.end() + 1:] # removes the css line

        block, paragraph = self.detab(block)
        if m:
            top_div = etree.SubElement(parent, 'div')
            p_el = self.applyElementSettings(etree.SubElement(top_div, 'p'), settings)
            p_el.text = paragraph
        else:
            top_div = sibling

        self.parser.parseChunk(top_div, block)


class CSSPreprocessor(Preprocessor, MooseMarkdownCommon):
    """
    Allows the !css syntax to work with lists and tables.
    """

    START_CHARS = ['1', '*', '-', '|']

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Preprocessor.__init__(self, markdown_instance)

    def run(self, lines):
        """
        Searches the raw markdown lines for '!css followed by a line beginning with '*', '-', or '1'
        and creates a div around the area if found.

        Args:
          lines[list]: List of markdown lines to preprocess.
        """
        content = '\n'.join(lines)
        content = re.sub(r'^(!css\s*(.*?)\n)(.*?)^$',
                         self._injectListCSS,
                         content,
                         flags=re.MULTILINE|re.DOTALL)
        return content.split('\n')

    def _injectListCSS(self, match):
        """
        Substitution function.
        """
        if match.group(3).strip()[0] in self.START_CHARS:
            settings = self.getSettings(match.group(2))
            string = ['{}={}'.format(key, str(value)) \
                      for key, value in settings.iteritems() if value]
            strt = self.markdown.htmlStash.store(u'<div {}>'.format(' '.join(string)), safe=True)
            stop = self.markdown.htmlStash.store(u'</div>', safe=True)
            return '\n\n{}\n\n{}\n\n{}\n\n'.format(strt, match.group(3), stop)
        return match.group(0)
