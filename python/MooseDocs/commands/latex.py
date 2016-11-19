import os
import re
import jinja2
import subprocess
import markdown
import markdown_include
import MooseDocs
from MooseDocs.html2latex import Translator, BasicExtension, MooseExtension


def latex_options(parser, subparser):
  """
  Command line arguments for "latex" command.

  Args:
    parser: The main argparse parser object.
    subparser: The main argparse subparser object.
  """

  tex = subparser.add_parser('latex', help='Generate a .tex or .pdf document from a markdown file.')
  tex.add_argument('input', type=str, help="The markdown file to convert to slides.")
  tex.add_argument('--template', type=str, default='basic.tex', help="The template tex file to utilize (default: %(default)s).")
  tex.add_argument('--output', '-o', default=None, help="The 'tex/pdf' file to create, if a .tex extension is provide only the latex will be created. If a pdf extension is provide than the pdf will be generated and all supporting files will be cleaned-up.")
  tex.add_argument('--site', default='http://mooseframework.com/docs/moose_docs/site', help='The website for where markdown links should be connected in latex/pdf file.')
  tex.add_argument('--hrule', type=bool, default=False, help='Disable the use use of \hrule in generated latex (default: %(default)s).')
  tex.add_argument('--headings', type=str, nargs=6, default=['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph'], help="The latex commands for the h1, h2, h3, h4, h5, and h6 tags for the document, all must be supplied and only commands valid in the latex body are allowed.")

  tex.add_argument('--documentclass', default='article', help="Set the contents of the \documentclass command (default: %(default)s).")
  tex.add_argument('--paper', default='letterpaper', help="Set the papersize to utilize (default: %(default)s).")
  tex.add_argument('--fontsize', default='12pt', help="Set the font size for the document (default: %(default)s).")
  tex.add_argument('--margin', default='1in', help="Set the document margins (default: %(default)s).")
  tex.add_argument('--linkcolor', default='blue', help="Set the hyperref package link color (default: %s(default).)")
  tex.add_argument('--tableofcontents', type=bool, default=True, help="Enable/disable the table of contents for the document (default: %(default)s).")
  tex.add_argument('--title', type=str, default=None, help="The title of the document.")
  tex.add_argument('--subtitle', type=str, default=None, help="The sub title of the document, require 'title' option.")
  tex.add_argument('--author', type=str, default=None, help="The author(s) to include on the titlepage, requires 'title' option.")
  tex.add_argument('--today', type=bool, default=True, help="Insert the current date on the titlepage, requires 'title' option.")
  tex.add_argument('--institution', type=str, default=None, help="Insert the institution on the titlepage, requires 'title' option.")

  return tex

def generate_html(input, config_file):
  """
  Generates html from Moose flavored markdown.

  Args:
    input[str]: The *.md file to convert.
    config_file[str]: The *.yml configuration file.
  """
  # Load the config to extract MooseMarkdown settings
  #TODO: Make this more robust
  config = MooseDocs.yaml_load(config_file)
  md_config = config['markdown_extensions'][-1]['MooseDocs.extensions.MooseMarkdown']
  md_config['dot_ext'] = 'svg'

  # Convert markdown
  with open(input, 'r') as fid:
    md = fid.read()

  # Extract Jinja2 blocks
  settings = dict()
  def sub(match):
    settings[match.group(1).strip()] = eval(match.group(2))
    return ''
  md = re.sub(r'@\+\s*set\s+(.*?)=(.*?)\+@', sub, md)

  moose = MooseDocs.extensions.MooseMarkdown(**md_config)
  parser = markdown.Markdown(extensions=[moose, 'markdown_include.include', 'admonition', 'mdx_math', 'toc', 'extra'])
  return parser.convert(md), settings


def generate_latex(html, **kwargs):
  """
  Generate latex from html.

  Args:
    key, value pairs are passed in from main latex command.
  """

  # The template .tex file
  template = kwargs.pop('template')

  # Options used by html2latex conversion
  config = dict()
  for option in ['hrule', 'site', 'headings']:
    config[option] = kwargs.pop(option)

  # Build latex
  h2l = Translator(extensions=[BasicExtension(**config), MooseExtension(**config)])
  tex = h2l.convert(html)

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
  cwd = os.path.dirname(tex_file)

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


def latex(config_file=None, output=None, input=None, **kwargs):
  """
  Command for converting markdown file to latex.
  """


  # Check that input is valid
  if not os.path.exists(input):
    raise Exception("The supplied input file does not exist: {}".format(input))

  # Determine the output file name
  if not output:
    output = os.path.splitext(input)[0] + '.pdf'

  # Build html
  html, settings = generate_html(input, config_file)

  # Merge settings from markdown file with arguments passed in to this command
  for key, value in kwargs.iteritems():
    if not value and key in settings:
      kwargs[key] = settings[key]

  # Generate latex
  tex = generate_latex(html, **kwargs)

  # Write tex file
  tex_file = output
  if output.endswith('.pdf'):
    tex_file = output.replace('.pdf', '.tex')
  with open(tex_file, 'w') as fid:
    fid.write(tex.encode('utf8'))

  # Create PDF
  if output.endswith('.pdf'):
    generate_pdf(tex_file, output)
