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
import re
import MooseDocs
from moose_docs_import import moose_docs_import
from nodes import DirectoryNode, MarkdownFileIndexNode, MarkdownFilePageNode, CopyFileNode


def finder(node, name):
    """Helper for finding child by name"""
    for child in node.children:
        if child.name == name:
            return child
    return None

def tree_builder(files, root, base, node, directory):
    """
    Helper for building markdown file tree.

    Inputs:
        files[set]:
    """
    for item in os.listdir(directory):

        # Complete path to the directory item (path or filename)
        path = os.path.join(directory, item)

        # Move along if path not in list of files
        if path in files:
            # Special case when the supplied node is the root, this maintains the root node
            # and creates an index node from which everything will stem.
            if item == 'index.md':
                if node.parent is None:
                    node = MarkdownFileIndexNode('', base=base, root_directory=root, parent=node)
                elif isinstance(node, DirectoryNode):
                    node = node.replace(MarkdownFileIndexNode(node.name, root_directory=root,
                                                              base=base))

            # General markdown files
            elif item.endswith('.md'):
                MarkdownFilePageNode(item[:-3], root_directory=root, base=base, parent=node)

            # Other files to copy
            elif item.endswith(MooseDocs.common.EXTENSIONS):
                CopyFileNode(item.lstrip('/'), root_directory=root, base=base, parent=node)

        # Directories
        elif os.path.isdir(path):
            n = finder(node, item)
            if n is None:
                n = DirectoryNode(item, base=base, parent=node)
            tree_builder(files, root, base, n, path)

def moose_docs_file_tree(config):
    """
    Creates a unified markdown file tree from multiple locations.

    Inputs:
        config[dict]: Contains key value pairs, with each value containing another dict() with
                      key value pairs that are passed to moose_docs_import function.
    """

    # Set the MOOSE_DIR if it does not exists so that the root_dir can always use it
    if 'MOOSE_DIR' not in os.environ:
        os.environ['MOOSE_DIR'] = MooseDocs.MOOSE_DIR

    # Build the file tree
    node = DirectoryNode('')
    for value in config.itervalues():
        value.setdefault('include', [])
        value.setdefault('exclude', [])
        value.setdefault('extensions', MooseDocs.common.EXTENSIONS)
        value.setdefault('base', '')
        value.setdefault('root_dir', MooseDocs.ROOT_DIR)
        value['root_dir'] = re.sub(r'\$(\w+)', lambda m: os.getenv(m.group(1)), value['root_dir'])
        if not os.path.isabs(value['root_dir']):
            value['root_dir'] = os.path.join(MooseDocs.ROOT_DIR, value['root_dir'])
        files = set(moose_docs_import(**value))
        tree_builder(files,
                     value['root_dir'],
                     value['base'],
                     node,
                     os.path.join(value['root_dir'], value['base']))

    # Remove un-used directories
    for desc in node.descendants:
        if isinstance(desc, DirectoryNode) and \
           all(isinstance(x, DirectoryNode) for x in desc.descendants):
            desc.parent = None

    return node
