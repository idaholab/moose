#!/usr/bin/env python
import os
import logging
import sys

import utils
import argparse

# Check for the necessary packages, this does a load so they should all get loaded.
if utils.check_configuration(['yaml', 'mkdocs', 'markdown', 'markdown_include', 'mdx_math']):
    sys.exit(1)

from mkdocs.commands import serve, build
from mkdocs.config import load_config
import MooseDocs

def command_line_options():
    """
    Return the command line options for the script.
    """

    # Command-line options
    parser = argparse.ArgumentParser(description="Tool for building and developing MOOSE and MOOSE-based application documentation.")
    parser.add_argument('--verbose', '-v', action='store_true', help="Execute with verbose (debug) output.")
    subparser = parser.add_subparsers(title='Commands', description="Documentation creation command to execute.", dest='command')

    # Generate options
    generate_parser = subparser.add_parser('generate', help="Generate the markdown documentation from MOOSE application executable. This is done by the serve and build command automatically.")
    generate_parser.add_argument('--moosedocs-config-file', type=str, default=os.path.join('moosedocs.yml'), help="The configuration file to use for building the documentation using MOOSE. (Default: %(default)s)")
    generate_parser.add_argument('--purge', action='store_true', help="Remove all content from the install directories.")

    # Serve options
    serve_parser = subparser.add_parser('serve', help='Generate and Sever the documentation using a local server.')
    serve_parser.add_argument('--livereload', dest='livereload', action='store_const', const='livereload', help="Enable the live reloading server.")
    serve_parser.add_argument('--dirtyreload', dest='livereload', action='store_const', const='dirtyreload', help="Enable the live reloading server without rebuilding entire site with single file change (default).")
    serve_parser.add_argument('--no-livereload', dest='livereload', action='store_const', const='no-livereload', help="Disable the live reloading of the served site.")
    serve_parser.add_argument('--strict', action='store_true', help='Enable strict mode and abort on warnings.')
    serve_parser.add_argument('--dirty', action='store_false', dest='clean', help='Do not clean the temporary build prior to building site.')

    # Build options
    build_parser = subparser.add_parser('build', help='Generate and Build the documentation for serving.')

    # Both build and serve need config file
    for p in [serve_parser, build_parser]:
        p.add_argument('--mkdocs-config-file', type=str, default=os.path.join('mkdocs.yml'), help="The configuration file to use for building the documentation using mkdocs. (Default: %(default)s)")
        p.add_argument('--theme', help="Build documentation using specified theme. The available themes are: cosmo, cyborg, readthedocs, yeti, journal, bootstrap, readable, united, simplex, flatly, spacelab, amelia, cerulean, slate, mkdocs")
        p.add_argument('--pages', default='pages.yml', help="YAML file containing the pages that are supplied to the mkdocs 'pages' configuration item.")

    # Parse the arguments
    options = parser.parse_args()

    # Set livereload default
    if options.command == 'serve' and not options.livereload:
        options.livereload = 'dirtyreload'

    return options


if __name__ == '__main__':

    # Options
    options = command_line_options()

    # Initialize logging
    formatter = MooseDocs.init_logging(options.verbose)

    # Execute command
    if options.command == 'generate':
        MooseDocs.commands.generate(config_file=options.moosedocs_config_file, purge=options.purge)
    elif options.command == 'serve':
        MooseDocs.commands.serve(config_file=options.mkdocs_config_file, strict=options.strict, livereload=options.livereload, clean=options.clean, theme=options.theme, pages=options.pages)
    elif options.command == 'build':
        MooseDocs.commands.build(config_file=options.mkdocs_config_file, theme=options.theme, pages=options.pages)

    # Display logging results
    print 'WARNINGS: {}  ERRORS: {}'.format(formatter.COUNTS['WARNING'], formatter.COUNTS['ERROR'])
    if formatter.COUNTS['ERROR'] > 0:
        sys.exit(1)
