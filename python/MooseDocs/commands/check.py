#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import os
import glob
import MooseDocs
from MooseDocs.MooseMarkdown import MooseMarkdown
from MooseDocs.extensions.app_syntax import AppSyntaxExtension

def get_default_groups():
    """
    Return the application group.
    """
    if MooseDocs.ROOT_DIR == MooseDocs.MOOSE_DIR:
        return []
    else:
        filenames = glob.glob(os.path.join(MooseDocs.ROOT_DIR, 'include', 'base', '*App.h'))
        if not filenames:
            return []
        return [os.path.basename(filenames[0][:-5]).lower()]
    return []

def check_options(parser):
    """
    Command-line options for check command.
    """
    parser.add_argument('--config-file', type=str, default='website.yml',
                        help="The configuration file to use for building the documentation using "
                             "MOOSE. (Default: %(default)s)")
    parser.add_argument('--template', type=str, default='website.html',
                        help="The template html file to utilize (default: %(default)s).")
    parser.add_argument('--generate', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files.")
    parser.add_argument('--update', action='store_true',
                        help="When checking the application for complete documentation generate "
                             "any missing markdown documentation files and update the stubs for "
                             "files that have not been modified.")
    parser.add_argument('--dump', action='store_true',
                        help="Dump the complete MooseDocs syntax tree to the screen.")
    parser.add_argument('--groups', default=get_default_groups(),
                        help="Specify the groups to consider in the check, by default only the "
                             "documentation for the application is considered, providing an empty "
                             "list will check all groups (default: %(default)s).")

def check(config_file=None, generate=None, update=None, dump=None, template=None, groups=None,
          **template_args):
    """
    Performs checks and optionally generates stub pages for missing documentation.
    """

    # Create the markdown parser and get the AppSyntaxExtension
    config = MooseDocs.load_config(config_file, template=template, template_args=template_args)
    parser = MooseMarkdown(config)
    ext = parser.getExtension(AppSyntaxExtension)
    syntax = ext.getMooseAppSyntax()

    # Dump the complete syntax tree if desired
    if dump:
        print syntax

    # Check all nodes for documentation
    for node in syntax.findall():
        node.check(ext.getConfig('install'), generate=generate, groups=groups, update=update)

    return 0
