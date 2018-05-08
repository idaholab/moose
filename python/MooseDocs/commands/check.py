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
                        help="Specify the groups to consider in the check, by default all groups "
                             "are considered.")

def check(translator, config, groups=None, dump=False, update=False, generate=False):
    """Helper to all both main and build.py:main to perfor check."""

    # Extract the syntax root node
    syntax = None
    extension = None
    for ext in translator.extensions:
        extension = ext
        if isinstance(ext, MooseDocs.extensions.appsyntax.AppSyntaxExtension):
            syntax = ext.syntax
            break

    if syntax is None:
        msg = "Failed to locate AppSyntaxExtension for the given configuration."
        raise exceptions.MooseDocsException(msg)

    # Use config.yml "Check:groups" if groups not provided
    if (groups is None) and ('Check' in config) and ('groups' in config['Check']):
        groups = config['Check']['groups']
    elif groups is None:
        groups = [extension.apptype]

    # Dump the complete syntax for the application
    if dump:
        print syntax

    # Perform check for all the nodes
    for node in anytree.PreOrderIter(syntax):
        node.check(generate=generate,
                   update=update,
                   groups=groups,
                   locations=config['Content'])

def main(options):
    """./moosedocs check"""

    translator, config = common.load_config(options.config)
    check(translator,
          config,
          groups=options.groups,
          dump=options.dump,
          update=options.update,
          generate=options.generate)
