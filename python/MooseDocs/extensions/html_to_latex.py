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

from markdown.postprocessors import Postprocessor

from MooseDocs.html2latex import Translator, BasicExtension, MooseExtension
from MooseMarkdownExtension import MooseMarkdownExtension

class HTML2LatexExtension(MooseMarkdownExtension):
    """
    Converts generated html to latex using MooseDocs.html2latex module.
    """
    @staticmethod
    def defaultConfig():
        """Default configuration for HTML2LatexExtension"""
        config = MooseMarkdownExtension.defaultConfig()
        config['site'] = ['', "The website for where markdown links should be connected in "
                              "latex/pdf file."]
        config['hrule'] = [False, "Enable/disable the use use of \\hrule in generated latex."]

        headings = ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph']
        config['headings'] = [headings, "The latex commands for the h1, h2, h3, h4, h5, and h6 "
                                        "tags for the document, all must be supplied and only "
                                        "commands valid in the latex body are allowed."]
        return config

    def extendMarkdown(self, md, md_globals):
        md.registerExtension(self)
        config = self.getConfigs()

        loc = '<moose_template' if 'moose_template' in md.postprocessors else '_end'
        md.postprocessors.add('moose_latex',
                              LatexPostprocessor(markdown_instance=md, **config),
                              loc)


def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create HTML2LatexExtension"""
    return HTML2LatexExtension(*args, **kwargs)

class LatexPostprocessor(Postprocessor):
    """
    Extension for converting html to latex.
    """
    def __init__(self, markdown_instance, **kwargs):
        super(LatexPostprocessor, self).__init__(markdown_instance)
        self._config = kwargs

    def run(self, text):
        """
        Converts supplied html to latex.
        """

        # Build latex
        h2l = Translator(extensions=[BasicExtension(**self._config),
                                     MooseExtension(**self._config)])
        tex = h2l.convert(text)
        return tex
