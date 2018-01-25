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

import sys
import collections

import logging
import json

import mooseutils
from nodes import SyntaxNode, MooseObjectNode, ActionNode, MooseObjectActionNode

LOG = logging.getLogger(__name__)

def add_moose_object_helper(name, item, parent):
    """
    Helper to handle the Postprocessor/UserObject and Bounds/AuxKernel special case.
    """
    node = MooseObjectNode(name, item, parent=parent)

    pairs = [('Postprocessor', 'UserObjects/*'), ('AuxKernel', 'Bounds/*')]
    for base, parent_syntax in pairs:
        if ('moose_base' in item) and (item['moose_base'] == base) and \
           (item['parent_syntax'] == parent_syntax):
            node.hidden = True

def syntax_tree_helper(item, parent):
    """
    Tree builder helper function.

    This investigates the JSON nodes and builds the proper input file tree for MooseDocs.
    """

    if item is None:
        return

    if 'actions' in item:
        for key, action in item['actions'].iteritems():
            if ('parameters' in action) and action['parameters'] and \
            ('isObjectAction' in action['parameters']):
                MooseObjectActionNode(key, action, parent=parent)
            else:
                ActionNode(key, action, parent=parent)

    if 'star' in item:
        syntax_tree_helper(item['star'], parent)

    if ('types' in item) and item['types']:
        for key, obj in item['types'].iteritems():
            add_moose_object_helper(key, obj, parent)

    if ('subblocks' in item) and item['subblocks']:
        for k, v in item['subblocks'].iteritems():
            node = SyntaxNode(k, parent=parent)
            syntax_tree_helper(v, node)

    if ('subblock_types' in item) and item['subblock_types']:
        for k, v in item['subblock_types'].iteritems():
            add_moose_object_helper(k, v, parent)


def moose_docs_app_syntax(location, hide=None):
    """
    Creates a tree structure representing the MooseApp syntax for the given executable.

    Inputs:
        location[str]: The folder to locate Moose executable.
        hide[dict]: Items to consider "hidden".
    """

    exe = mooseutils.find_moose_executable(location)
    if isinstance(exe, int):
        LOG.error("Unable to locate an executable in the supplied location: %s", location)
        sys.exit(1)

    try:
        raw = mooseutils.runExe(exe, ['--json', '--allow-test-objects'])
        raw = raw.split('**START JSON DATA**\n')[1]
        raw = raw.split('**END JSON DATA**')[0]
        tree = json.loads(raw, object_pairs_hook=collections.OrderedDict)
    except Exception: #pylint: disable=broad-except
        LOG.error("Failed to execute the MOOSE executable: %s", exe)
        sys.exit(1)

    root = SyntaxNode('')
    for key, value in tree['blocks'].iteritems():
        node = SyntaxNode(key, parent=root)
        syntax_tree_helper(value, node)

    if hide is not None:
        for node in root.findall():
            if ('all' in hide) and (node.full_name in hide['all']):
                node.hidden = True
            for group in node.groups:
                if (group in hide) and (node.full_name in hide[group]):
                    node.hidden = True
    return root
