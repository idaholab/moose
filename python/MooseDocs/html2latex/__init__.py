import os
import re
import jinja2
import subprocess
import markdown
import markdown_include
import logging
log = logging.getLogger(__name__)

import elements

import MooseDocs
from Translator import Translator
from BasicExtension import BasicExtension
from MooseExtension import MooseExtension
from MooseDocs.commands.MooseDocsMarkdownNode import MooseDocsMarkdownNode

def generate_html(md_file, config_file):
    """
    Generates html from Moose flavored markdown.

    Args:
      md_file[str]: The *.md file or text to convert.
      config_file[str]: The *.yml configuration file.
    """

    # Load the YAML configuration file
    config = MooseDocs.load_config(config_file, template=None)

    # Create the markdown parser
    extensions, extension_configs = MooseDocs.get_markdown_extensions(config)
    parser = MooseDocs.MooseMarkdown(extensions=extensions, extension_configs=extension_configs)

    # Read and parse the markdown
    md, settings = MooseDocs.read_markdown(md_file)
    return parser.convert(md), settings

def generate_latex(html, **kwargs):
    """
    Generate latex from html.

    Args:
      key, value pairs are passed in from main latex command.
    """

    # Options used by html2latex conversion
    config = dict()
    for option in ['hrule', 'site', 'headings']:
        config[option] = kwargs.pop(option)

    # Build latex
    h2l = Translator(extensions=[BasicExtension(**config), MooseExtension(**config)])
    tex = h2l.convert(html)
    return tex, h2l

def generate_latex_document(tex, h2l, **kwargs):
    """
    Create complete LaTeX document from template.
    """

    # The template .tex file
    template = kwargs.pop('template')

    # Build the latex preamble
    kwargs['preamble'] = h2l.preamble()

    env = jinja2.Environment(loader=jinja2.FileSystemLoader('templates'),
                 variable_start_string='++',
                 variable_end_string='++',
                 comment_start_string='%%',
                 comment_end_string='%%',
                 block_start_string='@+',
                 block_end_string='+@')
    template = env.get_template(template)
    return template.render(content=tex, **kwargs)

def generate_pdf(tex_file, output):
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
    for ext in ['.out', '.tex', '.aux', '.log', '.spl', '.bbl', '.toc', '.lof', '.lot', '.blg']:
        tmp = tex_file.replace('.tex', ext)
        if os.path.exists(tmp):
            os.remove(tmp)
