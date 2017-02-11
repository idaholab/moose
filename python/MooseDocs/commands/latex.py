import os
from MooseDocs import html2latex

def latex_options(parser):
    """
    Command line arguments for "latex" command.
    """

    parser.add_argument('input', type=str, help="The markdown file to convert to slides.")
    parser.add_argument('--config-file', type=str, default='moosedocs.yml', help="The configuration file to use for building the documentation using MOOSE. (Default: %(default)s)")
    parser.add_argument('--template', type=str, default='latex.tex', help="The template tex file to utilize (default: %(default)s).")
    parser.add_argument('--output', '-o', default=None, help="The 'tex/pdf' file to create, if a .tex extension is provide only the latex will be created. If a pdf extension is provide than the pdf will be generated and all supporting files will be cleaned-up.")
    parser.add_argument('--site', default='http://mooseframework.com/docs/moose_docs/site', help='The website for where markdown links should be connected in latex/pdf file.')
    parser.add_argument('--hrule', type=bool, default=False, help='Disable the use use of \hrule in generated latex (default: %(default)s).')
    parser.add_argument('--headings', type=str, nargs=6, default=['section', 'subsection', 'subsubsection', 'textbf', 'underline', 'emph'], help="The latex commands for the h1, h2, h3, h4, h5, and h6 tags for the document, all must be supplied and only commands valid in the latex body are allowed.")

    parser.add_argument('--documentclass', default='article', help="Set the contents of the \documentclass command (default: %(default)s).")
    parser.add_argument('--paper', default='letterpaper', help="Set the papersize to utilize (default: %(default)s).")
    parser.add_argument('--fontsize', default='12pt', help="Set the font size for the document (default: %(default)s).")
    parser.add_argument('--margin', default='1in', help="Set the document margins (default: %(default)s).")
    parser.add_argument('--linkcolor', default='blue', help="Set the hyperref package link color (default: %s(default).)")
    parser.add_argument('--tableofcontents', type=bool, default=True, help="Enable/disable the table of contents for the document (default: %(default)s).")
    parser.add_argument('--title', type=str, default=None, help="The title of the document.")
    parser.add_argument('--subtitle', type=str, default=None, help="The sub title of the document, require 'title' option.")
    parser.add_argument('--author', type=str, default=None, help="The author(s) to include on the titlepage, requires 'title' option.")
    parser.add_argument('--today', type=bool, default=True, help="Insert the current date on the titlepage, requires 'title' option.")
    parser.add_argument('--institution', type=str, default=None, help="Insert the institution on the titlepage, requires 'title' option.")

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
    html, settings = html2latex.generate_html(input, config_file)

    # Merge settings from markdown file with arguments passed in to this command
    for key, value in kwargs.iteritems():
        if not value and key in settings:
            kwargs[key] = settings[key]

    # Generate latex
    tex, h2l = html2latex.generate_latex(html, **kwargs)
    tex = html2latex.generate_latex_document(tex, h2l, **kwargs)

    # Write tex file
    tex_file = output
    if output.endswith('.pdf'):
        tex_file = output.replace('.pdf', '.tex')
    with open(tex_file, 'w') as fid:
        fid.write(tex.encode('utf8'))

    # Create PDF
    if output.endswith('.pdf'):
        html2latex.generate_pdf(tex_file, output)
