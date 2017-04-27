import os
import sys
import re
import argparse
import subprocess
import multiprocessing

import extensions
import commands
import mooseutils

# Check for the necessary packages, this does a load so they should all get loaded.
if mooseutils.check_configuration(['yaml', 'jinja2', 'markdown', 'markdown_include', 'mdx_math', 'bs4']):
    sys.exit(1)

import yaml
from MarkdownTable import MarkdownTable
from MooseApplicationSyntax import MooseApplicationSyntax
from MooseLinkDatabase import MooseLinkDatabase

import logging
log = logging.getLogger(__name__)
log.addHandler(logging.NullHandler())

MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')

ROOT_DIR = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'], stderr=subprocess.STDOUT).strip('\n')

class MooseDocsFormatter(logging.Formatter):
    """
    A formatter that is aware of the class hierarchy of the MooseDocs library.

    Call the init_logging function to initialize the use of this custom formatter.
    """
    COLOR = {'DEBUG':'CYAN', 'INFO':'RESET', 'WARNING':'YELLOW', 'ERROR':'RED', 'CRITICAL':'MAGENTA'}
    COUNTS = {'ERROR': multiprocessing.Value('I', 0, lock=True), 'WARNING': multiprocessing.Value('I', 0, lock=True)}

    def format(self, record):
        msg = logging.Formatter.format(self, record)
        if record.levelname in self.COLOR:
            msg = mooseutils.colorText(msg, self.COLOR[record.levelname])

        if record.levelname in self.COUNTS:
            with self.COUNTS[record.levelname].get_lock():
                self.COUNTS[record.levelname].value += 1

        return msg

    def counts(self):
        return self.COUNTS['WARNING'].value, self.COUNTS['ERROR'].value


def abspath(*args):
    """
    Create an absolute path from paths that are given relative to the ROOT_DIR.

    Inputs:
      *args: Path(s) that are defined relative to the git repository root directory as defined in ROOT_DIR
    """
    return os.path.abspath(os.path.join(ROOT_DIR, *args))


def relpath(abs_path):
    """
    Create a relative path from the absolute path given relative to the ROOT_DIR.

    Inputs:
      abs_path[str]: Absolute path that to be converted to a relative path to the git repository root directory as defined in ROOT_DIR
    """
    return os.path.relpath(abs_path, ROOT_DIR)


def init_logging(verbose=False):
    """
    Call this function to initialize the MooseDocs logging formatter.
    """

    # Setup the logger object
    if verbose:
        level = logging.DEBUG
    else:
        level = logging.INFO

    # Custom format that colors and counts errors/warnings
    formatter = MooseDocsFormatter()
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)

    # The markdown package dumps way too much information in debug mode (so always set it to INFO)
    log = logging.getLogger('MARKDOWN')
    log.setLevel(logging.INFO)

    # Setup the custom formatter
    log = logging.getLogger('MooseDocs')
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
        filename = abspath(self.construct_scalar(node))
        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)
        else:
            raise IOError("Unknown included file: {}".format(filename))


def yaml_load(filename, loader=Loader):
    """
    Load a YAML file capable of including other YAML files.

    Args:
      filename[str]: The name to the file to load, relative to the git root directory
      loader[yaml.Loader]: The loader to utilize.
    """

    # Attach the include constructor to our custom loader.
    Loader.add_constructor('!include', Loader.include)

    if not os.path.exists(filename):
        raise IOError("The supplied configuration file was not found: {}".format(filename))

    with open(filename, 'r') as fid:
        yml = yaml.load(fid.read(), Loader)

    return yml

def load_config(config_file, **kwargs):
    """
    Read the MooseDocs configure file (e.g., moosedocs.yml)
    """
    config = yaml_load(config_file)
    config.update(kwargs)

    # Set the default arguments
    config.setdefault('navigation', abspath(os.path.join(os.getcwd(), 'navigation.yml')))
    config.setdefault('template', 'materialize.html')
    config.setdefault('template_arguments', dict())
    config.setdefault('markdown_extensions', [])
    return config

def get_markdown_extensions(config):
    """
    Extract the markdown extensions and associated configurations from the yaml configuration.
    """
    extensions = []
    extension_configs = dict()
    for extension in config['markdown_extensions']:
        if isinstance(extension, dict):
            for k, v in extension.iteritems(): # there should only be one entry, but just in case
                extensions.append(k)
                extension_configs[k] = v
        else:
            extensions.append(extension)

    return extensions, extension_configs

def get_moose_markdown_extension(parser):
    """
    Return the MooseMarkdown instance from the Markdown parser, if it exists.
    """
    for ext in parser.registeredExtensions:
        if isinstance(ext, extensions.MooseMarkdown):
            return ext


def read_markdown(md_file):
    """
    Reads and removes meta data (key, value pairs) from the top of a markdown file.

    Inputs:
      md_file[str]: The *.md file or text to convert.
    """

    # Read file, if provided
    if os.path.isfile(md_file):
        with open(md_file, 'r') as fid:
            content = fid.read().decode('utf-8')
    else:
        content = md_file

    # Extract meta data
    output = dict()
    count = 0
    lines = content.splitlines()
    for line in lines:
        if line == '':
            break
        match = re.search(r'^(?P<key>[A-Za-z0-9_-]+)\s*:\s*(?P<value>.*)', line)
        if match:
            try:
                value = eval(match.group('value'))
            except:
                value = match.group('value')
            output[match.group('key')] = value
            count += 1
        else:
            break
    return '\n'.join(lines[count:]), output

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


def command_line_options(*args):
    """
    Return the command line options for the moosedocs script.
    """

    # Command-line options
    parser = argparse.ArgumentParser(description="Tool for building and developing MOOSE and MOOSE-based application documentation.")
    parser.add_argument('--verbose', '-v', action='store_true', help="Execute with verbose (debug) output.")

    subparser = parser.add_subparsers(title='Commands', description="Documentation creation command to execute.", dest='command')

    # Add the sub-commands
    test_parser = subparser.add_parser('test', help='Performs unit testing of MooseDocs module.')
    commands.test_options(test_parser)

    check_parser = subparser.add_parser('check', help="Check that the documentation exists and is complete for your application and optionally generating missing markdown files.")
    commands.check_options(check_parser)

    build_parser = subparser.add_parser('build', help='Build the documentation for serving on another system.')
    commands.build_options(build_parser)

    serve_parser = subparser.add_parser('serve', help='Serve the documentation using a local server.')
    commands.serve_options(serve_parser)

    latex_parser = subparser.add_parser('latex', help='Generate a .tex or .pdf document from a markdown file.')
    commands.latex_options(latex_parser)

    presentation_parser = subparser.add_parser('presentation', help="Convert a markdown file to an html presentation.")
    commands.presentation_options(presentation_parser)

    return parser.parse_args(*args)

def moosedocs():

    # Options
    options = vars(command_line_options())

    # Initialize logging
    formatter = init_logging(options.pop('verbose'))
    log = logging.getLogger('MooseDocs')

    # Remove moose.svg files (these get generated via dot)
    log.debug('Removing *.moose.svg files from {}'.format(os.getcwd()))
    purge(['svg'])

    # Pull LFS image files
    try:
        subprocess.check_output(['git', 'lfs', 'pull'])
    except subprocess.CalledProcessError:
        print "ERROR: Unable to run 'git lfs', it is likely not installed but required for the MOOSE documentation system."
        sys.exit(1)

    # Execute command
    cmd = options.pop('command')
    if cmd == 'test':
        retcode = commands.test(**options)
    elif cmd == 'check':
        retcode = commands.check(**options)
    elif cmd == 'generate':
        retcode = commands.generate(**options)
    elif cmd == 'serve':
        retcode = commands.serve(**options)
    elif cmd == 'build':
        retcode = commands.build(**options)
    elif cmd == 'latex':
        retcode = commands.latex(**options)
    elif cmd == 'presentation':
        retcode = commands.presentation(**options)

    # Check retcode
    if retcode is not None:
        return retcode

    # Display logging results
    warn, err = formatter.counts()
    print 'WARNINGS: {}  ERRORS: {}'.format(warn, err)
    return err > 0
