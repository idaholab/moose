import os
import sys
import argparse
import argparse
import logging

import extensions
import database
import commands
import utils

# Check for the necessary packages, this does a load so they should all get loaded.
if utils.check_configuration(['yaml', 'mkdocs', 'markdown', 'markdown_include', 'mdx_math']):
    sys.exit(1)

import yaml
import mkdocs
from mkdocs.commands import serve, build

from MarkdownTable import MarkdownTable
from MooseObjectParameterTable import MooseObjectParameterTable
from MooseApplicationSyntax import MooseApplicationSyntax

import logging
logging.getLogger(__name__).addHandler(logging.NullHandler())

MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')

class MkMooseDocsFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.

    Call the init_logging function to initialize the use of this custom formatter.
    """
    COLOR = {'DEBUG':'CYAN', 'INFO':'RESET', 'WARNING':'YELLOW', 'ERROR':'RED', 'CRITICAL':'MAGENTA'}
    COUNTS = {'DEBUG':0, 'INFO':0, 'WARNING':0, 'ERROR':0, 'CRITICAL':0}

    def format(self, record):
        msg = logging.Formatter.format(self, record)

        if record.name.endswith('Item'):
            level = 3
        elif record.name.endswith('Database'):
            level = 2
        elif record.name.endswith('MooseApplicationSyntax') or record.name.endswith('MooseCommonFunctions'):
            level = 1
        else:
            level = 0

        if record.levelname in ['DEBUG', 'WARNING', 'ERROR', 'CRITICAL']:
            msg = '{}{}: {}'.format(' '*4*level, record.levelname, msg)
        else:
            msg = '{}{}'.format(' '*4*level, msg)

        if record.levelname in self.COLOR:
            msg = utils.colorText(msg, self.COLOR[record.levelname])

        # Increment counts
        self.COUNTS[record.levelname] += 1

        return msg

def init_logging(verbose=False):
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Setup the logger object
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.INFO

    # The markdown package dumps way too much information in debug mode (so always set it to INFO)
    log = logging.getLogger('MARKDOWN')
    log.setLevel(logging.INFO)

    # Setup the custom formatter
    log = logging.getLogger('MooseDocs')
    formatter = MkMooseDocsFormatter()
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    log.addHandler(handler)
    log.setLevel(level)

    log = logging.getLogger('mkdocs')
    log.addHandler(handler)
    log.setLevel(level)

    return formatter

class Loader(yaml.Loader):
    """
    A custom loader that handles nested includes. The nested includes should use absolute paths from the
    origin yaml file.
    """

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        http://stackoverflow.com/questions/528281/how-can-i-include-an-yaml-file-inside-another
        """
        filename = self.construct_scalar(node)
        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)

def yaml_load(filename, loader=Loader):
    """
    Load a YAML file capable of including other YAML files.

    Args:
        filename[str]: The name fo the file to load.b
        loader[yaml.Loader]: The loader to utilize.
    """

    ## Attach the include constructor to our custom loader.
    Loader.add_constructor('!include', Loader.include)

    with open(filename, 'r') as fid:
        yml = yaml.load(fid.read(), Loader)

    return yml

def load_pages(filename, keys=[], **kwargs):
    """
    A YAML loader for reading the pages file.

    Args:
        filename[str]: The name for the file to load. This is normally a '.yml' file that contains the complete
                       website layout. It may also be a markdown file, in this case only that single file will be
                       served with the "home" page.
        keys[list]: A list of top-level keys to include.
        kwargs: key, value pairs passed to yaml_load function.
    """

    if filename.endswith('.md'):
        pages = [{'Home':'index.md'}, {os.path.basename(filename):filename}]

    else:
        pages = yaml_load(filename, **kwargs)

    # Restrict the top-level keys to those provided in the 'include' argument
    if keys:
        pages = [page for page in pages if page.keys()[0] in keys]

    return pages

def purge(extensions):
    """
    Removes generated files from repository.

    Args:
        extensions[list]: List of file extensions to purge (.e.g., 'png'); it will be prefixed with '.moose.'
                          so the files actually removed are '.moose.png'.
    """
    for i, ext in enumerate(extensions):
        extensions[i] = '.moose.{}'.format(ext)

    log = logging.getLogger('MooseDocs')
    for root, dirs, files in os.walk(os.getcwd(), topdown=False):
        for name in files:
            if any([name.endswith(ext) for ext in extensions]):
                full_file = os.path.join(root, name)
                log.debug('Removing: {}'.format(full_file))
                os.remove(full_file)

def command_line_options():
    """
    Return the command line options for the moosedocs script.
    """

    # Command-line options
    parser = argparse.ArgumentParser(description="Tool for building and developing MOOSE and MOOSE-based application documentation.")
    parser.add_argument('--verbose', '-v', action='store_true', help="Execute with verbose (debug) output.")
    parser.add_argument('--config-file', type=str, default=os.path.join('moosedocs.yml'), help="The configuration file to use for building the documentation using MOOSE. (Default: %(default)s)")

    subparser = parser.add_subparsers(title='Commands', description="Documentation creation command to execute.", dest='command')

    # Check
    check_parser = subparser.add_parser('check', help="Perform error checking on documentation.")

    # Generate options
    generate_parser = subparser.add_parser('generate', help="Check that documentation exists for your application and generate the markdown documentation from MOOSE application executable.")
    generate_parser.add_argument('--no-stubs', dest='stubs', action='store_false', help="Disable the creation of system and object stub markdown files.")
    generate_parser.add_argument('--no-pages-stubs', dest='pages_stubs', action='store_false', help="Disable the creation the pages.yml files.")

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
        p.add_argument('--theme', help="Build documentation using specified theme. The available themes are: cosmo, cyborg, readthedocs, yeti, journal, bootstrap, readable, united, simplex, flatly, spacelab, amelia, cerulean, slate, mkdocs")
        p.add_argument('--pages', default='pages.yml', help="YAML file containing the pages that are supplied to the mkdocs 'pages' configuration item. It also supports passing the name of a single markdown file, in this case only this file will be served with the 'Home' page.")
        p.add_argument('--page-keys', default=[], nargs='+', help='A list of top-level keys from the "pages" file to include. This is a tool to help speed up the serving for development of documentation.')

    # Parse the arguments
    options = parser.parse_args()

    # Set livereload default
    if options.command == 'serve' and not options.livereload:
        options.livereload = 'dirtyreload'

    return options

def moosedocs():

    # Options
    options = command_line_options()

    # Initialize logging
    formatter = init_logging(options.verbose)
    log = logging.getLogger('MooseDocs')

    # Remove moose.svg files (these get generated via dot)
    log.info('Removing *.moose.svg files from {}'.format(os.getcwd()))
    purge(['svg'])

    # Execute command
    if options.command == 'check':
        commands.generate(config_file=options.config_file, stubs=False, pages_stubs=False)
    elif options.command == 'generate':
        commands.generate(config_file=options.config_file, stubs=options.stubs, pages_stubs=options.pages_stubs)
    elif options.command == 'serve':
        commands.serve(config_file=options.config_file, strict=options.strict, livereload=options.livereload, clean=options.clean, theme=options.theme, pages=options.pages, page_keys=options.page_keys)
    elif options.command == 'build':
        commands.build(config_file=options.config_file, theme=options.theme, pages=options.pages, page_keys=options.page_keys)

    # Display logging results
    print 'WARNINGS: {}  ERRORS: {}'.format(formatter.COUNTS['WARNING'], formatter.COUNTS['ERROR'])
    return formatter.COUNTS['ERROR'] > 0
