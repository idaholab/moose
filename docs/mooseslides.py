#!/usr/bin/env python
import os
import re
import argparse
import markdown
import markdown_include
import MooseDocs

def command_line_options():
    """
    Return the command line options for the slide generator.
    """
    parser = argparse.ArgumentParser(description="Tool for building slides from MOOSE flavored markdown.")
    parser.add_argument('input', type=str, help="The markdown file to convert to slides.")
    parser.add_argument('output', nargs='?', default=None, help="The name of the output file.")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    options = command_line_options()

    # Error if MOOSE_REVEAL_JS_DIR is not defined
    if 'MOOSE_REVEAL_JS_DIR' not in os.environ:
        raise Exception("The location of the reveal.js library must be given in the MOOSE_REVEAL_JS_DIR environment variable.")

    # Check that filename exists
    if not os.path.exists(options.input):
        raise Exception("The file {} does not exists.".format(options.input))

    # Define the output file
    if not options.output:
        filename, _ = os.path.splitext(options.input)
        options.output = '{}.html'.format(filename)

    # Read the markdown and convert to html
    with open(options.input, 'r') as fid:
        md = fid.read()
    parser = markdown.Markdown(extensions=[MooseDocs.extensions.MooseMarkdown(slides=True), 'markdown_include.include', 'smarty', 'admonition', 'mdx_math'])
    html = parser.convert(md)

    # Read the reveal.js index.html, the markdown will be injected into this file.
    index = os.path.join(os.path.dirname(__file__), 'slides', 'template.html')
    if not os.path.exists(index):
        raise Exception('Failed to locate the the reveal.js/index.html file.')
    with open(index, 'r') as fid:
        complete = fid.read()

    # Apply the environment variable
    complete = complete.replace('$MOOSE_REVEAL_JS_DIR', os.path.relpath(os.environ['MOOSE_REVEAL_JS_DIR'], os.path.dirname(options.output)))

    # Inject the code and write output
    complete = complete.replace("INSERT_MOOSE_SLIDES_HERE", html)
    with open(options.output, 'w') as fid:
        fid.write(complete)
