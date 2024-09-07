#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import collections
import logging
import json

import moosetree
import mooseutils
from .nodes import SyntaxNode, MooseObjectNode, ActionNode, MooseObjectActionNode

def get_moose_syntax_tree(exe, remove=None, alias=None, unregister=None, markdown=None):
    """
    Creates a tree structure representing the MooseApp syntax for the given executable using --json.

    Inputs:
      ext[str|dict]: The executable to run or the parsed JSON tree structure
      remove[list|dict]: Syntax to mark as removed. The input data structure can be a single list or
                         a dict of lists.
      alias[dict]: A dict of alias information; the key is the actual syntax and the value is the
                   alias to be applied (e.g., {'/Kernels/Diffusion':'/Physics/Diffusion'}).
      unregister[dict]: A dict (or dict of dict) of classes with duplicate registration information;
                        the key is the "moose_base" name and the value is the syntax from which the
                        object should be removed (e.g., {"Postprocessor":"UserObject/*"}).
    """
    # Create the JSON tree, unless it is provided directly
    if isinstance(exe, dict):
        tree = exe
    else:
        raw = mooseutils.runExe(exe, ['--json', '--allow-test-objects'])
        if (len(raw.split('**START JSON DATA**\n')) == 1):
            logging.getLogger('MooseSyntax.GetSyntaxTree').error("JSON syntax file creation failed. Do you have a working executable built?")
        raw = raw.split('**START JSON DATA**\n')[1]
        raw = raw.split('**END JSON DATA**')[0]
        tree = mooseutils.json_parse(raw)

    # Build the complete syntax tree
    root = SyntaxNode(None, '')
    for key, value in tree['blocks'].items():
        node = SyntaxNode(root, key)
        __syntax_tree_helper(node, value)

    # Build/initialize inputs for use
    removed = __build_set_from_yaml(remove)
    unregister = __build_dict_from_yaml(unregister or dict())
    alias = alias or dict()
    markdown = __build_dict_from_yaml(markdown or dict())

    # Apply remove/alias/unregister restrictions
    for node in moosetree.iterate(root):

        # Removed
        if (node.fullpath() in removed) or ((node.parent is not None) and node.parent.removed):
            node.removed = True

        # Remove unregistered items
        for base, parent_syntax in unregister.items():
            if (node.name == base) and (node.get('action_path') == parent_syntax):
                node.removed = True

            if (node.get('moose_base') == base) and (node.get('parent_syntax') == parent_syntax):
                node.removed = True

        # Apply alias
        for name, alt in alias.items():
            if node.fullpath() == name:
                node.alias = str(alt)

        # Mark 'Test' objects
        if node.groups() and all(grp.endswith('TestApp') for grp in node.groups()):
            node.test =  True

        # Explicitly set markdown files
        if (node.fullpath() in markdown):
            node.markdown = markdown[node.fullpath()]

    return root

def __build_set_from_yaml(item):
    """Helper for converting list/dict structure from YAML file to single set."""
    out = set()
    if isinstance(item, dict):
        for value in item.values():
            out.update(value)
    elif isinstance(item, (list, set)):
        out.update(item)
    return out

def __build_dict_from_yaml(item):
    """Helper for converting dict of dict structure from YAML file to single dict."""
    out = dict()
    for key, value in item.items():
        if isinstance(value, dict):
            out.update(value)
        else:
            out[key] = value
    return out

def __syntax_tree_helper(parent, item):
    """Helper to build the proper node from the supplied JSON item."""

    if item is None:
        return

    if 'actions' in item:
        for key, action in item['actions'].items():
            action['tasks'] = set(action['tasks'])
            if ('parameters' in action) and action['parameters'] and \
            ('isObjectAction' in action['parameters']):
                MooseObjectActionNode(parent, key, **action)
            else:
                ActionNode(parent, key, **action)

    if 'star' in item:
        __syntax_tree_helper(parent, item['star'])

    if ('types' in item) and item['types']:
        for key, obj in item['types'].items():
            MooseObjectNode(parent, key, **obj)

    if ('subblocks' in item) and item['subblocks']:
        for k, v in item['subblocks'].items():
            node = SyntaxNode(parent, k)
            __syntax_tree_helper(node, v)

    if ('subblock_types' in item) and item['subblock_types']:
        for k, v in item['subblock_types'].items():
            MooseObjectNode(parent, k, **v)
