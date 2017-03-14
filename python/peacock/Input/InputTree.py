#!/usr/bin/env python
from InputFile import InputFile
import mooseutils
import InputTreeWriter
import os

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

    def setInputFile(self, input_filename):
        """
        The input file has changed.
        Input:
            input_filename[str]: The name of the input file
        Return:
            bool: If it was a valid input file
        """
        try:
            # if the user passes in a bad input file, then leave
            # things untouched
            new_input_file = InputFile(input_filename)
            self.input_file = new_input_file
        except Exception:
            return False

        self.input_filename = input_filename
        if self.app_info.valid():
            self._copyDefaultTree()
            self.root.comments = '\n'.join(self.input_file.root_node.comments)

            for root_node in self.input_file.getTopNodes():
                self._addInputFileNode(root_node)
                if root_node not in self.root.children_write_first:
                    self.root.children_write_first.append(root_node.name)
            return True
        return False

    def _addInputFileNode(self, input_node):
        """
        This adds a node from the input file.
        Input:
            input_node[GPNode]: The node from the input file.
        """
        path = input_node.fullName(no_root=True)
        entry = self.path_map.get(path)
        if not entry:
            parent_path = os.path.dirname(path)
            name = os.path.basename(path)
            entry = self.addUserBlock(parent_path, name)
            if not entry:
                mooseutils.mooseWarning("Could not create %s" % path)
                return

        entry.comments = "\n".join(input_node.comments)
        entry.included = True
        entry.hard = True

        in_type_name = input_node.params.get("type")
        param_info = entry.parameters.get("type")
        if in_type_name and param_info:
            entry.setBlockType(in_type_name)
            param_info.value = in_type_name
            if "type" not in entry.parameters_list:
                entry.parameters_list.insert(0, "type")

        for param_name in input_node.params_list:
            param_value = input_node.params[param_name]
            param_info = self.getParamInfo(path, param_name)
            if not param_info:
                # must be a user added param
                param_info = self.addUserParam(path, param_name, param_value)
            else:
                param_info.value = param_value
            param_info.set_in_input_file = True
            if param_info.name not in param_info.parent.parameters_write_first:
                param_info.parent.parameters_write_first.append(param_info.name)
            param_info.comments = input_node.param_comments.get(param_name, "")

        for child_name in input_node.children_list:
            child_node = input_node.children[child_name]
            self._addInputFileNode(child_node)
            if child_name not in entry.children_write_first:
                entry.children_write_first.append(child_name)

    def getInputFileString(self):
        """
        Convert the input file to a string.
        Return:
            str of the entire input file.
        """
        return InputTreeWriter.inputTreeToString(self.root)

    def addUserBlock(self, parent, name):
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
