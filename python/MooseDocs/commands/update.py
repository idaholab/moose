#pylint: disable=missing-docstring, no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import logging
import subprocess
import anytree

import MooseDocs
from MooseDocs.tree import syntax
from MooseDocs import common


LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'update' command."""

    parser = subparser.add_parser('update',
                                  parents=[parent],
                                  help='Syntax update tool for MooseDocs.')
    parser.add_argument('--config', default='config.yml', help="The configuration file.")

def main(options):
    """./moosedocs update"""

    # Load syntax
    translator, _ = common.load_config(options.config)
    root = None
    for ext in translator.extensions:
        if isinstance(ext, MooseDocs.extensions.appsyntax.AppSyntaxExtension):
            root = ext.syntax
            break

    for node in anytree.PreOrderIter(root, filter_=lambda n: isinstance(n, syntax.ObjectNode)):

        idx = node.source().find('/src/')
        old = os.path.join(node.source()[:idx], 'doc', 'content', 'documentation', 'systems',
                           node.fullpath.lstrip('/') + '.md')

        if not os.path.isfile(old):
            continue

        new = os.path.join(node.source()[:idx], 'doc', 'content', 'source', node.markdown())
        loc = os.path.dirname(new)
        if not os.path.isdir(loc):
            os.makedirs(loc)
        cmd = ['git', 'mv', '--verbose', old, new]
        subprocess.call(cmd)

    func = lambda n: isinstance(n, syntax.SyntaxNode) and not n.is_root
    for node in anytree.PreOrderIter(root, filter_=func):
        action = anytree.search.findall(node, filter_=lambda n: isinstance(n, syntax.ActionNode))[0]
        idx = action.source().find('/src/')
        old = os.path.join(action.source()[:idx], 'doc', 'content', 'documentation', 'systems',
                           os.path.dirname(node.markdown()), 'index.md')

        if not os.path.isfile(old):
            continue

        new = os.path.join(action.source()[:idx], 'doc', 'content', 'syntax',
                           os.path.dirname(node.markdown()), 'index.md')

        loc = os.path.dirname(new)
        if not os.path.isdir(loc):
            os.makedirs(loc)
        cmd = ['git', 'mv', '--verbose', old, new]
        subprocess.call(cmd)
