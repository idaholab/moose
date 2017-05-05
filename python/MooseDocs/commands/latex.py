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

import os
import logging
import subprocess

import MooseDocs
from MarkdownNode import MarkdownNode

LOG = logging.getLogger(__name__)

def latex_options(parser):
    """
    Command line arguments for "latex" command.
    """

    parser.add_argument('md_file', type=str, help="The markdown file to convert to slides.")
    parser.add_argument('--config-file', type=str, default='latex.yml',
                        help="The configuration file to use for building the documentation using "
                             "MOOSE. (Default: %(default)s)")
    parser.add_argument('--template', type=str, default='latex.tex',
                        help="The template tex file to utilize (default: %(default)s).")
    parser.add_argument('--output', '-o', default=None,
                        help="The 'tex/pdf' file to create, if a .tex extension is provide only "
                             "the latex will be created. If a pdf extension is provide than the "
                             "pdf will be generated and all supporting files will be cleaned-up.")
    parser.add_argument('--site', default='http://mooseframework.com/docs/moose_docs/site',
                        help="The website for where markdown links should be connected in "
                             "latex/pdf file.")
    parser.add_argument('--hrule', type=bool, default=False,
                        help="Disable the use use of \\hrule in generated latex "
                             "(default: %(default)s).")

    heading_defaults = ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph']
    parser.add_argument('--headings', type=str, nargs=6, default=heading_defaults,
                        help="The latex commands for the h1, h2, h3, h4, h5, and h6 tags for the "
                             "document, all must be supplied and only commands valid in the latex "
                             "body are allowed.")
    parser.add_argument('--documentclass', default='article',
                        help="Set the contents of the \\documentclass command "
                             "(default: %(default)s).")
    parser.add_argument('--paper', default='letterpaper',
                        help="Set the papersize to utilize (default: %(default)s).")
    parser.add_argument('--fontsize', default='12pt',
                        help="Set the font size for the document (default: %(default)s).")
    parser.add_argument('--margin', default='1in',
                        help="Set the document margins (default: %(default)s).")
    parser.add_argument('--linkcolor', default='blue',
                        help="Set the hyperref package link color (default: %s(default).)")
    parser.add_argument('--tableofcontents', type=bool, default=True,
                        help="Enable/disable the table of contents for the document "
                             "(default: %(default)s).")
    parser.add_argument('--title', type=str, default=None, help="The title of the document.")
    parser.add_argument('--subtitle', type=str, default=None,
                        help="The sub title of the document, require 'title' option.")
    parser.add_argument('--author', type=str, default=None,
                        help="The author(s) to include on the titlepage, requires 'title' option.")
    parser.add_argument('--today', type=bool, default=True,
                        help="Insert the current date on the titlepage, requires 'title' option.")
    parser.add_argument('--institution', type=str, default=None,
                        help="Insert the institution on the titlepage, requires 'title' option.")

class LatexBuilder(MarkdownNode):
    """
    Creates TeX and/or PDF from markdown.
    """
    def __init__(self, output=None, **kwargs):
        super(LatexBuilder, self).__init__(**kwargs)
        self._output = output if output else self.source().replace('.md', '.pdf')
        self._tex_output = self._output.replace('.pdf', '.tex')

    def write(self, *args, **kwargs):
        """
        Adds PDF creation to write command.
        """
        super(LatexBuilder, self).write(*args, **kwargs)

        if self._output.endswith('.pdf'):
            self.generatePDF(self._tex_output)

    def url(self, parent=None): #pylint: disable=unused-argument
        """
        Path to the tex file to be created.
        """
        return self._tex_output

    def path(self):
        """
        Return the *.tex file to be creted.
        """
        return os.path.dirname(self._tex_output)

    @staticmethod
    def generatePDF(tex_file):
        """
        Create the PDF file using pdflatex and bibtex.
        """

        # Working directory
        cwd = os.path.abspath(os.path.dirname(tex_file))

        # Call pdflatex
        local_file = os.path.basename(tex_file)

        subprocess.call(["pdflatex", local_file], cwd=cwd)
        subprocess.call(["bibtex", os.path.splitext(local_file)[0]], cwd=cwd)
        subprocess.call(["pdflatex", local_file], cwd=cwd)
        subprocess.call(["pdflatex", local_file], cwd=cwd)

        # Clean-up
        for ext in ['.out', '.aux', '.LOG', '.spl', '.bbl', '.toc', '.lof', '.lot', '.blg']:
            tmp = tex_file.replace('.tex', ext)
            if os.path.exists(tmp):
                os.remove(tmp)

def latex(config_file=None, output=None, md_file=None, **kwargs):
    """
    Command for converting markdown file to latex.
    """

    # Load the YAML configuration file
    config = MooseDocs.load_config(config_file, **kwargs)
    parser = MooseDocs.MooseMarkdown(extensions=config.keys(), extension_configs=config)

    site_dir, _ = os.path.splitext(md_file)
    root = LatexBuilder(output, name='', markdown=md_file, parser=parser, site_dir=site_dir)
    root.build()
    return None
