"""Developer tools for MooseDocs."""
import logging

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'check' command."""

    parser = subparser.add_parser('check',
                                  parents=[parent],
                                  help='Syntax checking tools for MooseDocs.')

    parser.add_argument('--config', default='config.yml', help="The configuration file.")
    parser.add_argument('--generate', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files.")
    parser.add_argument('--update', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files and update the stubs for "
                             "files that have not been modified.")
    parser.add_argument('--dump', action='store_true',
                        help="Dump the complete MooseDocs syntax tree to the screen.")

def check(translator, dump=False, update=False, generate=False):
    """Helper to all both main and build.py:main to perform check."""

    # Extract the syntax root node
    syntax = None
    extension = None
    for ext in translator.extensions:
        extension = ext
        if isinstance(ext, MooseDocs.extensions.appsyntax.AppSyntaxExtension):
            syntax = ext.syntax
            break

    if extension['disable']:
        LOG.info("Syntax is disabled, skipping the check.")
        return

    if syntax is None:
        msg = "Failed to locate AppSyntaxExtension for the given configuration."
        raise exceptions.MooseDocsException(msg)

    # Dump the complete syntax for the application
    if dump:
        print syntax

    # Perform check for all the nodes
    group = extension.apptype
    for node in anytree.PreOrderIter(syntax):
        node.check(generate=generate,
                   update=update,
                   group=group,
                   root=translator.root)

def main(options):
    """./moosedocs check"""

    translator, _ = common.load_config(options.config)
    check(translator,
          dump=options.dump,
          update=options.update,
          generate=options.generate)
