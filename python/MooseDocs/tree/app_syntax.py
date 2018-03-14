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

import anytree

import mooseutils

from MooseDocs import common
from MooseDocs.tree.syntax import SyntaxNode, MooseObjectNode, ActionNode, MooseObjectActionNode

LOG = logging.getLogger(__name__)


REGISTER_PAIRS = [('Postprocessor', 'UserObjects/*'),
                  ('AuxKernel', 'Bounds/*')]

def app_syntax(exe, remove=None, remove_test_apps=True, hide=None):
    """
    Creates a tree structure representing the MooseApp syntax for the given executable.
    """
    common.check_type('exe', exe, str)
    common.check_type('remove', remove, (type(None), dict, list, set))
    common.check_type('hide', hide, (type(None), dict, list, set))
    common.check_type('remove_test_apps', remove_test_apps, bool)

    try:
        raw = mooseutils.runExe(exe, ['--json', '--allow-test-objects'])
        raw = raw.split('**START JSON DATA**\n')[1]
        raw = raw.split('**END JSON DATA**')[0]
        tree = json.loads(raw, object_pairs_hook=collections.OrderedDict)
        raw = mooseutils.runExe(exe, ['--registry-hit'])
        raw = raw.split('### START REGISTRY DATA ###\n')[1]
        raw = raw.split('### END REGISTRY DATA ###')[0]
        reg = mooseutils.hit_load(raw)

    except Exception as e: #pylint: disable=broad-except
        LOG.error("Failed to execute the MOOSE executable '%s':\n%s", exe, e.message)
        sys.exit(1)

    root = SyntaxNode(None, '')
    for key, value in tree['blocks'].iteritems():
        node = SyntaxNode(root, key)
        __syntax_tree_helper(node, value)

    hidden = set()
    if isinstance(hide, dict):
        for value in hide.itervalues():
            hidden.update(value)
    elif isinstance(hide, (list, set)):
        hidden.update(hide)

    if hidden:
        for node in anytree.PreOrderIter(root):
            if node.fullpath in hidden:
                node.hidden = True

    #TODO: When the new registration methods are added to the --json dump, this will not be needed.
    # Add groups from --registry-hit output
    object_groups = collections.defaultdict(set)
    action_groups = collections.defaultdict(set)

    for node in reg.children[0].children[0]:
        object_groups[node['name']].add(node['label'].replace('App', ''))

    for node in reg.children[0].children[1]:
        action_groups[node['name']].add(node['label'].replace('App', ''))
        action_groups[node['class']].add(node['label'].replace('App', ''))

    for node in anytree.PreOrderIter(root):
        if isinstance(node, MooseObjectNode):
            node._groups.update(object_groups[node.name]) #pylint: disable=protected-access
        elif isinstance(node, ActionNode):
            node._groups.update(action_groups[node.name]) #pylint: disable=protected-access
            for task in node.tasks:
                node._groups.update(action_groups[task]) #pylint: disable=protected-access

    removed = set()
    if isinstance(remove, dict):
        for value in remove.itervalues():
            removed.update(value)
    elif isinstance(remove, (list, set)):
        removed.update(remove)

    if removed:
        for node in anytree.PreOrderIter(root):
            if any(n.fullpath == prefix for n in node.path for prefix in removed):
                node.removed = True

    if remove_test_apps:
        for node in anytree.PreOrderIter(root):
            if all([group.endswith('Test') for group in node.groups]):
                node.removed = True

    return root

def __add_moose_object_helper(parent, name, item):
    """
    Helper to handle the Postprocessor/UserObject and Bounds/AuxKernel special case.
    """
    node = MooseObjectNode(parent, name, item)

    for base, parent_syntax in REGISTER_PAIRS:
        if ('moose_base' in item) and (item['moose_base'] == base) and \
           (item['parent_syntax'] == parent_syntax):
            node.removed = True

def __syntax_tree_helper(parent, item):
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
                MooseObjectActionNode(parent, key, action)
            else:
                ActionNode(parent, key, action)

    if 'star' in item:
        __syntax_tree_helper(parent, item['star'])

    if ('types' in item) and item['types']:
        for key, obj in item['types'].iteritems():
            __add_moose_object_helper(parent, key, obj)

    if ('subblocks' in item) and item['subblocks']:
        for k, v in item['subblocks'].iteritems():
            node = SyntaxNode(parent, k)
            __syntax_tree_helper(node, v)

    if ('subblock_types' in item) and item['subblock_types']:
        for k, v in item['subblock_types'].iteritems():
            __add_moose_object_helper(parent, k, v)
