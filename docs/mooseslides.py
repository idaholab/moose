#!/usr/bin/env python
import os
import re
import shutil
import argparse
import markdown
import markdown_include
import MooseDocs

def command_line_options():
    """
    Return the command line options for the slide generator.
    """
    reveal_default = os.path.join(os.getenv('HOME'), 'projects', 'reveal.js')

    parser = argparse.ArgumentParser(description="Tool for building slides from MOOSE flavored markdown.")
    parser.add_argument('input', type=str, help="The markdown file to convert to slides.")
    parser.add_argument('--reveal', '-r', default=reveal_default, help="The name of the reveal.js location where this presentation is to be installed (default: %(default)s).")
    parser.add_argument('--output', '-o', default='index.html', help="The default html file to create within the revel directory (default: %(default)s).")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    options = command_line_options()

    # Check that the reveal.js exists and contains reveal.js
    if not os.path.exists(options.reveal):
        raise Exception("The reveal.js directory {} does not exist.".format(options.reveal))
    elif not os.path.exists(os.path.join(options.reveal, 'js', 'reveal.js')):
        raise Exception("The reveal.js directory {} does not seem to be a clone of the reveal.js repository.".format(options.reveal))

    # Check that filename exists
    if not os.path.exists(options.input):
        raise Exception("The file {} does not exists.".format(options.input))

    # Copy moose.css
    src = os.path.join(os.path.dirname(__file__), 'css', 'moose.css')
    dst = os.path.join(options.reveal, 'css', 'moose.css')
    shutil.copyfile(src, dst)

    # Read the markdown and convert to html
    with open(options.input, 'r') as fid:
        md = fid.read()

    parser = markdown.Markdown(extensions=[MooseDocs.extensions.MooseMarkdown(slides=True), 'markdown_include.include', 'smarty', 'admonition', 'mdx_math', 'toc'])
    html = parser.convert(md)

    # Read the reveal.js index.html, the markdown will be injected into this file.
    index = os.path.join(os.path.dirname(__file__), 'slides', 'template.html')
    if not os.path.exists(index):
        raise Exception('Failed to locate the the reveal.js/index.html file.')
    with open(index, 'r') as fid:
        complete = fid.read()

    # Inject the code and write output
    complete = complete.replace("INSERT_MOOSE_SLIDES_HERE", html)
    with open(os.path.join(options.reveal, options.output), 'w') as fid:
        fid.write(complete)
