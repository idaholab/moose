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

import markdown

class MooseMarkdownExtension(markdown.Extension):
    """
    A wrapper class to define a static method for extracting the default configure options.

    This is need so that the default options can be displayed via a table using the devel extension
    without getting the configuration settings actually being used.
    """

    @staticmethod
    def defaultConfig():
        """
        Return a dict() containing the default extension configuration options.
        """
        return dict()

    def __init__(self, *args, **kwargs):
        self.config = self.defaultConfig()
        super(MooseMarkdownExtension, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        super(MooseMarkdownExtension, self).extendMarkdown(md, md_globals)
