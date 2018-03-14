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
import re
import logging
import subprocess
import anytree

import mooseutils

import MooseDocs
from MooseDocs.tree import syntax, app_syntax

LOG = logging.getLogger(__name__)

def command_line_options(subparser, parent):
    """Define the 'update' command."""

    parser = subparser.add_parser('update',
                                  parents=[parent],
                                  help='Syntax update tool for MooseDocs.')
    parser.add_argument('--executable', '-e', type=str, default='..',
                        help="The application executable to utilize for update.")

    parser.add_argument('--dirname', default=os.path.join('content', 'documentation', 'systems'),
                        type=str, help="The root directory of documentation.")

    parser.add_argument('--move', action='store_true',
                        help="Enables call to 'git mv' for moving files to the new locations.")

    parser.add_argument('--update', action='store_true',
                        help="Enables automatic syntax updating.")

def main(options):
    """./moosedocs update"""

    # Load syntax
    exe = os.path.abspath(os.path.join(os.getcwd(), options.executable))
    if os.path.isdir(exe):
        exe = mooseutils.find_moose_executable(exe)
    LOG.info("Loading application syntax: %s", exe)
    root = app_syntax(exe)

    # Determine group name
    group_dir = os.path.basename(exe).split('-')[0]
    group = group_dir.replace('_', ' ').title().replace(' ', '')

    if options.move:
        for node in anytree.PreOrderIter(root):
            if isinstance(node, syntax.ObjectNode) and group in node.groups:
                _move(node, group_dir, options)

    if options.update:
        for root, _, files in os.walk(MooseDocs.ROOT_DIR):
            for name in files:
                if name.endswith('.md'):
                    filename = os.path.join(root, name)
                    print 'Update:', filename
                    _update(filename)


def _move(node, group_dir, options):
    new = os.path.abspath(os.path.join(options.dirname, node.markdown()))
    old = new.split('/')
    old.insert(-1, group_dir)
    old = '/' + os.path.join(*old)

    print old
    if os.path.exists(old):
        print "{}:\n    OLD: {}\n    NEW: {}\n".format(node.name, old, new)
        subprocess.call(['git', 'mv', old, new])

def _update(filename):

    with open(filename, 'r') as fid:
        content = fid.read()

    # Headings
    content = re.sub(r'^(#+?)(\w.*?)$', r'\1 \2', content, flags=re.MULTILINE)
    content = re.sub(r'^(#+ .*?)$(?=\n^\S)', r'\1\n', content, flags=re.MULTILINE)

    # Commands
    content = re.sub(r'(?<=\S)(\n)(?=^!\w+)', r'\n\n', content, flags=re.MULTILINE)
    content = re.sub(r'^(!.*?\n)(?=\S)', r'\1\n', content, flags=re.MULTILINE)

    # Format
    content = re.sub(r'(?<=\s)\*{2}(.*?)\*{2}(?=\s)', r'+\1+', content, flags=re.MULTILINE)

    #Syntax
    content = re.sub(r'^!syntax objects (\S+)',
                     r'!syntax list \1 objects=True actions=False subsystems=False', content,
                     flags=re.MULTILINE)
    content = re.sub(r'^!syntax actions (\S+)',
                     r'!syntax list \1 objects=False actions=True subsystems=False', content,
                     flags=re.MULTILINE)
    content = re.sub(r'^!syntax subsystems (\S+)',
                     r'!syntax list \1 objects=False actions=False subsystems=True', content,
                     flags=re.MULTILINE)
    content = re.sub(r'^!listing\s*(.*?) *label=\w+ *(.*?)$', r'!listing \1 \2', content,
                     flags=re.MULTILINE)
    content = re.sub(r'^!syntax list\s*(.*?) *title=\w+ *(.*?)$', r'!syntax list \1 \2', content,
                     flags=re.MULTILINE)

    # Fig/Eq
    content = re.sub(r'(Fig\.\s)(?=\\ref)', r'', content)
    content = re.sub(r'(Eq\.\s)(?=\\eqref)', r'', content)
    content = re.sub(r'\\eqref\{(.*?)\}', r'[\1]', content)

    # Bibtex
    content = re.sub(r'\\(cite|citet|citep)\{(.*?)\}', r'[\1:\2]', content)
    content = re.sub(r'\\ref\{(.*?)\}', r'[\1]', content)
    content = re.sub(r'\\bibliographystyle\{.*?\}', '!bibtex bibliography', content)
    content = re.sub(r'\\bibliography\{.*?\}\n', '', content)
    content = re.sub(r'^##\s*References$', '', content, flags=re.MULTILINE)
    content = re.sub(r'^!bibtex$', '!bibtex bibliogrpahy', content, flags=re.MULTILINE)
    content = re.sub(r'^!bibtex bibliogrpahy$', '!bibtex bibliography', content, flags=re.MULTILINE)

    # Misc.
    content = re.sub(r'suffix=(\S+)', 'footer=\1', content)
    content = re.sub(r'include_end=(\S+)', 'include-end=\1', content)

    with open(filename, 'w') as fid:
        content = fid.write(content)
