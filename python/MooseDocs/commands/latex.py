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

import os
import logging
import subprocess

import MooseDocs
from MooseDocs.MooseMarkdown import MooseMarkdown
from MooseDocs.common import nodes, Builder

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

    # The following are passed in as template arguments
    t_args = parser.add_argument_group('Template Arguments', "The following options are passed "
                                                             "directly to the jinja2 template "
                                                             "supplied in the --template option.")
    t_args.add_argument('--site', default='http://mooseframework.com/docs/moose_docs/site',
                        help="The website for where markdown links should be connected in "
                             "latex/pdf file.")
    t_args.add_argument('--hrule', type=bool, default=False,
                        help="Disable the use use of \\hrule in generated latex "
                             "(default: %(default)s).")

    heading_defaults = ['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph']
    t_args.add_argument('--headings', type=str, nargs=6, default=heading_defaults,
                        help="The latex commands for the h1, h2, h3, h4, h5, and h6 tags for the "
                             "document, all must be supplied and only commands valid in the latex "
                             "body are allowed.")
    t_args.add_argument('--documentclass', default='article',
                        help="Set the contents of the \\documentclass command "
                             "(default: %(default)s).")
    t_args.add_argument('--paper', default='letterpaper',
                        help="Set the papersize to utilize (default: %(default)s).")
    t_args.add_argument('--fontsize', default='12pt',
                        help="Set the font size for the document (default: %(default)s).")
    t_args.add_argument('--margin', default='1in',
                        help="Set the document margins (default: %(default)s).")
    t_args.add_argument('--linkcolor', default='blue',
                        help="Set the hyperref package link color (default: %s(default).)")
    t_args.add_argument('--tableofcontents', type=bool, default=True,
                        help="Enable/disable the table of contents for the document "
                             "(default: %(default)s).")
    t_args.add_argument('--title', type=str, default=None, help="The title of the document.")
    t_args.add_argument('--subtitle', type=str, default=None,
                        help="The sub title of the document, require 'title' option.")
    t_args.add_argument('--author', type=str, default=None,
                        help="The author(s) to include on the titlepage, requires 'title' option.")
    t_args.add_argument('--today', type=bool, default=True,
                        help="Insert the current date on the titlepage, requires 'title' option.")
    t_args.add_argument('--institution', type=str, default=None,
                        help="Insert the institution on the titlepage, requires 'title' option.")

class MarkdownLatexNode(nodes.MarkdownFilePageNode):
    """
    A node based on a single markdown file for building LaTeX.

    Inputs:
        markdown[str]: The markdown file to convert.
    """
    def __init__(self, markdown):
        name = os.path.basename(markdown)[:-3]
        base = os.path.dirname(markdown)
        super(MarkdownLatexNode, self).__init__(name, base=base)

    @property
    def destination(self):
        return self.basename + '.tex'

class LatexBuilder(Builder):
    """
    Creates TeX and/or PDF from markdown.
    """
    def __init__(self, md_file=None, output=None, **kwargs):
        self._output = output if output else md_file.replace('.md', '.pdf')
        self._md_file = md_file

        kwargs['site_dir'] = os.path.join(MooseDocs.ROOT_DIR, os.path.dirname(self._output))
        super(LatexBuilder, self).__init__(**kwargs)

    def buildNodes(self):
        """
        Create the markdown node objects to build.
        """
        # Check if file exists
        full = os.path.join(MooseDocs.ROOT_DIR, self._md_file)
        if not os.path.isfile(full):
            raise IOError("The presentation markdown file does not exist: {}.".format(full))

        # Return the node for converting.
        return MarkdownLatexNode(self._md_file)

    def write(self, *args, **kwargs):
        """
        Adds PDF creation to write command.
        """
        super(LatexBuilder, self).write(*args, **kwargs)
        if self._output.endswith('.pdf'):
            self.generatePDF(self._root.destination)

    def copyFiles(self):
        """
        There is nothing to copy, but this must be implemented.
        """
        pass

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

def latex(config_file=None, output=None, md_file=None, template=None, **template_args):
    """
    Command for converting markdown file to latex.
    """
    LOG.warning("The latex command is experimental and requires additional development to be "
                "complete, please be patient as this develops.\nIf you would like to aid in "
                "improving this feature please contact the MOOSE mailing list: "
                "moose-users@googlegroups.com.")

    # The markdown file is provided via the command line, thus it is provided relative to the
    # current working directory. The internals of MooseDocs are setup to always work from the
    # repository root directory (i.e., MooseDocs.ROOT_DIR), thus the path of this file must be
    # converted to be relative to MooseDocs.ROOT_DIR.
    md_file = os.path.relpath(os.path.abspath(md_file), MooseDocs.ROOT_DIR)

    # Create the markdown parser
    config = MooseDocs.load_config(config_file, template=template, template_args=template_args)
    parser = MooseMarkdown(config)

    # Build the html
    builder = LatexBuilder(md_file=md_file, output=output, parser=parser)
    builder.init()
    builder.build(num_threads=1)
    return 0
