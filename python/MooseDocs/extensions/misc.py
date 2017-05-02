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

from markdown.treeprocessors import Treeprocessor
from markdown.util import etree

from MooseMarkdownExtension import MooseMarkdownExtension

class MiscExtension(MooseMarkdownExtension):
    """
    Extension for adding materialize specific css to converted markdown.
    """
    @staticmethod
    def defaultConfig():
        """Configuration for MiscExtension"""
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds materialize specific css to the converted html.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.treeprocessors.add('moose_content_scroll',
                              ScrollContents(markdown_instance=md, **config), '_end')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create MiscExtension"""
    return MiscExtension(*args, **kwargs)

class ScrollContents(Treeprocessor):
    """
    Adds a 'div' tag around h2 levels with class of 'section scrollspy' to allow scrollable contents
    on right-hand side of pages.
    """

    def __init__(self, markdown_instance=None, **kwargs): #pylint: disable=unused-argument
        super(ScrollContents, self).__init__(markdown_instance)

    def run(self, root):
        """
        Adds section for materialize scrollspy
        """

        section = root
        for el in list(root):
            if el.tag == 'h2':
                section = etree.Element('div', id=el.get('id', '#'))
                section.set('class', "section scrollspy")
                root.append(section)
            root.remove(el)
            section.append(el)
