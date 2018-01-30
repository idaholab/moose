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
