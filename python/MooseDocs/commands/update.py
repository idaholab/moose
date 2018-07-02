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

import mooseutils

from MooseDocs.tree import syntax, app_syntax

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'update' command."""

    parser = subparser.add_parser('update',
                                  parents=[parent],
                                  help='Syntax update tool for MooseDocs.')
    parser.add_argument('--executable', '-e', type=str, default='..',
                        help="The application executable to utilize for update.")
    parser.add_argument('--move', action='store_true', help="Move markdown files to new location.")

def main(options):
    """./moosedocs update"""

    # Move files
    if options.move:
        _move(options)

def _move(options):

    # Load syntax
    exe = os.path.abspath(os.path.join(os.getcwd(), options.executable))
    if os.path.isdir(exe):
        exe = mooseutils.find_moose_executable(exe)
    LOG.info("Loading application syntax: %s", exe)
    root = app_syntax(exe)

    if options.move:
        LOG.info('Moving markdown files:')
    else:
        LOG.info('Moving markdown files (dry-run, use --move to perform actual command):')

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
