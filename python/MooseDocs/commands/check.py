"""Developer tools for MooseDocs."""
import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions

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
    parser.add_argument('--groups',
                        nargs='*',
                        default=None,
                        help="Specify the groups to consider in the check, by default only the "
                             "groups for the application is considered.")

def main(options):
    """./moosedocs check"""

    translator = common.load_config(options.config)

    # Extract the syntax root node
    syntax = None
    for ext in translator.extensions:
        if isinstance(ext, MooseDocs.extensions.appsyntax.AppSyntaxExtension):
            syntax = ext.syntax
            break

    if syntax is None:
        msg = "Failed to locate AppSyntaxExtension for the given configuration."
        raise exceptions.MooseDocsException(msg)

    # Dump the complete syntax for the application
    if options.dump:
        print syntax

    # Perform check for all the nodes
    for node in anytree.PreOrderIter(syntax):
        node.check(generate=options.generate, update=options.update, groups=options.groups)
