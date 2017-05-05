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
from markdown.preprocessors import Preprocessor
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

class GlobalExtension(MooseMarkdownExtension):
    """
    Extension for adding global markdown style link ids.
    """
    @staticmethod
    def defaultConfig():
        """Default GlobalExtension configuration options"""
        config = MooseMarkdownExtension.defaultConfig()
        config['globals'] = ['', "List of global markdown links (e.g., [foo]: bar)."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds Bibtex support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose_globals',
                             GlobalPreprocessor(markdown_instance=md, **config), '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create GlobalExtension"""
    return GlobalExtension(*args, **kwargs)

class GlobalPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    Appends global links to markdown content
    """
    @staticmethod
    def defaultSettings():
        """GlobalPreprocessor settings"""
        return dict() # this extension doesn't have settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Preprocessor.__init__(self, markdown_instance)
        self._globals = kwargs.pop('globals', dict())

    def run(self, lines):
        """
        Append globals
        """
        return lines + ['[{}]: {}'.format(key, value) for key, value in self._globals.iteritems()]
