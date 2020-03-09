#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from pyhit import hit
import mooseutils
from .InputFile import InputFile
from . import InputTreeWriter

class InputTree(object):
    """
    A tree that represents an input file along with all the available blocks and parameters.
    """
    def __init__(self, app_info, **kwds):
        """
        Constructor.
        Input:
            app_info[ExecutableInfo]: Where to get default data from.
        """
        super(InputTree, self).__init__(**kwds)
        self.app_info = app_info
        self.input_file = None
        self.input_filename = None
        self._copyDefaultTree()
        self.root = None
        self.path_map = {}
        self.input_has_errors = False
        if self.app_info.valid():
            self._copyDefaultTree()

    def getBlockInfo(self, path):
        return self.path_map.get(path)

    def getParamInfo(self, path, param):
        info = self.getBlockInfo(path)
        if info:
            return info.getParamInfo(param)

    def _addToPathMap(self, node):
        if node.hard:
            self.path_map[node.path] = node
            for c in node.children.values():
                self._addToPathMap(c)

    def _copyDefaultTree(self):
        if self.app_info.valid():
            root = self.app_info.path_map["/"]
            self.root = root.copy(None)
            self.path_map = {}
            self.root.hard = True
            self._addToPathMap(self.root)

    def resetInputFile(self):
        self.input_filename = None
        self.path_map = {}
        self.root = None
        if self.app_info.valid():
            self._copyDefaultTree()

    def _getComments(self, node):
        """
        Get the comments for a node
        """
        comments = []
        for n in node.children(node_type=hit.NodeType.Comment):
            c = n.render().strip()[1:].strip()
            comments.append(c)

        return '\n'.join(comments)

    def setInputFileData(self, input_str, filename="String"):
        """
        Set a new input file based on a existing string
        Input:
            input_str[str]: The input file data to parse
        Return:
            bool: If it was a valid input file
        """
        try:
            new_input_file = InputFile()
            if filename is None:
                filename = ""
            new_input_file.readInputData(input_str, filename)
            return self._setInputFile(new_input_file)
        except Exception as e:
            mooseutils.mooseWarning("setInputFileData exception: %s" % e)
            return False

    def setInputFile(self, input_filename):
        """
        Set a new input file
        Input:
            input_filename[str]: The name of the input file
        Return:
            bool: If it was a valid input file
        """
        try:
            new_input_file = InputFile(input_filename)
            return self._setInputFile(new_input_file)
        except Exception as e:
            mooseutils.mooseWarning("setInputFile exception: %s" % e)
            return False

    def _setInputFile(self, input_file):
        """
        Copies the nodes of an input file into the tree
        Input:
            input_file[InputFile]: Input file to copy
        Return:
            bool: True if successfull
        """
        self.input_has_errors = False
        if not input_file.root_node:
            return False
        self.input_file = input_file
        self.input_filename = input_file.filename

        if self.app_info.valid():
            self._copyDefaultTree()
            self.root.comments = self._getComments(self.input_file.root_node)
            active = self._readParameters(self.input_file.root_node, "/")
            for root_node in self.input_file.root_node.children(node_type=hit.NodeType.Section):
                self._addInputFileNode(root_node, root_node.path() in active)
                if root_node.path() not in self.root.children_write_first:
                    self.root.children_write_first.append(root_node.path())
            return True
        return False

    def _readParameters(self, input_node, path):
        """
        Read the parameters set on a block.
        We special case the "active" and "inactive" parameters and
        convert them directly into whether we include the block or
        not. Peacock will write out the "inactive" parameter for
        those blocks that are deselected and we don't want to have
        any problems with existing active or inactive parameters.
        Input:
            input_node[hit.Node]: Input node to read parameters from
            path[str]: Full path of the input_node
        Return:
            list[str]: Children of this node that are active
        """
        active = []
        for child_node in input_node.children(node_type=hit.NodeType.Section):
            active.append(child_node.path())

        for param_node in input_node.children(node_type=hit.NodeType.Field):
            param_info = self.getParamInfo(path, param_node.path())
            if not param_info:
                # must be a user added param
                param_info = self.addUserParam(path, param_node.path(), param_node.raw())
            else:
                param_info.value = param_node.raw()
            if param_info.name == "active":
                active = param_info.value.split()
                param_info.parent.removeUserParam("active", True)
            elif param_info.name == "inactive":
                for inactive in param_info.value.split():
                    try:
                        active.remove(inactive)
                    except ValueError:
                        pass
                # We don't want to write this out by default
                param_info.parent.removeUserParam("inactive", True)
            else:
                param_info.set_in_input_file = True
                param_info.parent.changed_by_user = True
                if param_info.name not in param_info.parent.parameters_write_first:
                    param_info.parent.parameters_write_first.append(param_info.name)
                param_info.comments = self._getComments(param_node)
        return active

    def _addInputFileNode(self, input_node, included):
        """
        This adds a node from the input file.
        Input:
            input_node[hit.Node]: The node from the input file.
        """
        path = "/" + input_node.fullpath()
        entry = self.path_map.get(path)
        if not entry:
            parent_path = os.path.dirname(path)
            name = os.path.basename(path)
            entry = self.addUserBlock(parent_path, name)
            if not entry:
                mooseutils.mooseWarning("Could not create %s" % path)
                self.input_has_errors = True
                return

        entry.comments = self._getComments(input_node)
        entry.included = included
        entry.hard = True

        in_type = input_node.find("type")
        param_info = entry.parameters.get("type")
        if in_type and in_type.raw() and param_info:
            entry.setBlockType(in_type.raw())
            param_info.value = in_type.raw()
            if "type" not in entry.parameters_list:
                entry.parameters_list.insert(0, "type")

        active = self._readParameters(input_node, path)

        for child_node in input_node.children(node_type=hit.NodeType.Section):
            self._addInputFileNode(child_node, child_node.path() in active)
            if child_node.path() not in entry.children_write_first:
                entry.children_write_first.append(child_node.path())

    def getInputFileString(self):
        """
        Convert the input file to a string.
        Return:
            str of the entire input file.
        """
        if not self.root:
            return ""
        return InputTreeWriter.inputTreeToString(self.root)

    def addUserBlock(self, parent, name):
        if not parent:
            parent = "/"
        info = self.path_map.get(parent)
        if info and info.star_node:
            block = self._copyNode(info.path, info, info.star_node, name)
            block.user_added = True
            return block

    def _addChildrenToPaths(self, parent):
        for c in parent.children.values():
            self.path_map[c.path] = c
            self._addChildrenToPaths(c)

    def _copyNode(self, parent_path, new_parent, node, name):
        new_entry = node.copy(node)
        new_entry.name = name
        new_entry.path = os.path.join(parent_path, name)
        new_parent.addChildBlock(new_entry)
        self.path_map[new_entry.path] = new_entry
        self._addChildrenToPaths(new_entry)
        return new_entry

    def cloneUserBlock(self, clone_from, new_name):
        info = self.path_map.get(clone_from)
        if info:
            block = self._copyNode(info.parent.path, info.parent, info, new_name)
            return block

    def removeUserBlock(self, parent, name):
        full_path = os.path.join(parent, name)
        self.removeBlock(full_path)

    def _removeChildPaths(self, parent):
        for c in parent.children.values():
            if c.path in self.path_map:
                del self.path_map[c.path]

    def renameUserBlock(self, parent, oldname, newname):
        old_full_path = os.path.join(parent, oldname)
        info = self.path_map.get(old_full_path)
        if info:
            del self.path_map[old_full_path]
            self._removeChildPaths(info)
            info.parent.renameChildBlock(oldname, newname)
            self.path_map[info.path] = info
            self._addChildrenToPaths(info)

    def setBlockType(self, parent, new_type):
        """
        Changes the block type
        """
        info = self.path_map.get(parent)
        if info:
            info.setBlockType(new_type)

    def setBlockSelected(self, path, included):
        info = self.path_map.get(path)
        info.included = included

    def addUserParam(self, parent, param, value):
        parent_entry = self.path_map.get(parent)
        if parent_entry:
            return parent_entry.addUserParam(param, value)

    def removeBlock(self, path):
        info = self.path_map.get(path)
        if info:
            del self.path_map[path]
            info.parent.removeChildBlock(info.name)
            self._removeChildPaths(info)

    def moveBlock(self, path, new_index):
        """
        Moves the block to another position
        """
        pinfo = self.path_map.get(path)
        if pinfo:
            pinfo.parent.moveChildBlock(pinfo.name, new_index)

    def incompatibleChanges(self, app_info):
        """
        Tries to detect if there are any incompatible changes
        in the syntax with the current working input file.
        Input:
            app_info[ExecutableInfo]: The executable info to compare against
        Return:
            bool: True if errors occurred loading the current input file with the new syntax
        """

        if not self.app_info.json_data:
            return False
        if self.app_info.json_data and not app_info.json_data:
            return True

        try:
            old_input = self.getInputFileString()
            new_tree = InputTree(app_info)
            read_data = new_tree.setInputFileData(old_input)
            return not read_data or new_tree.input_has_errors
        except Exception as e:
            mooseutils.mooseWarning("Caught exception: %s" % e)
            return True

if __name__ == '__main__':
    import sys
    from ExecutableInfo import ExecutableInfo
    if len(sys.argv) < 3:
        print("Usage: <path_to_exe> <path_to_input_file>")
        exit(1)
    exe_info = ExecutableInfo()
    exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    input_file_path = sys.argv[2]
    input_tree = InputTree(exe_info)
    input_tree.setInputFile(input_file_path)
    #print(input_tree.dump())
    print(input_tree.getInputFileString())
