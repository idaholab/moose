#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import copy
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO
import mooseutils
from .ParameterInfo import ParameterInfo

class BlockInfo(object):
    """
    Holds information about a block.
    """
    def __init__(self, parent, path, hard=False, description=""):
        """
        Input:
            parent[BlockInfo]: Parent of this block
            path[str]: Path of this block
            hard[bool]: Whether this is a hard path.
            description[str]: Description of this block.
        """
        super(BlockInfo, self).__init__()
        self.parameters = {}
        self.parameters_list = []
        self.parameters_write_first = []
        self.user_added = False
        self.star = False
        self.star_node = None
        self.types = {}
        self.included = False
        self.comments = ""
        self.name = os.path.basename(path)
        self.path = path
        self.children = {}
        self.children_list = []
        self.children_write_first = []
        self.hard = hard
        self.description = ""
        self.parent = parent
        self.changed_by_user = False

    def checkInactive(self):
        return not self.included and self.wantsToSave()

    def wantsToSave(self):
        return self.changed_by_user or self.user_added or self.included or self.childrenWantToSave()

    def childrenWantToSave(self):
        for key in self.children_list:
            if self.children[key].wantsToSave():
                return True
        return False

    def getParamInfo(self, param):
        """
        Gets a ParameterInfo with the given name.
        This looks in the current type if applicable.
        Input:
            param[str]: Parameter name
        Return:
            ParameterInfo if found else None
        """
        param_info = self.parameters.get(param)
        if param_info:
            return param_info

        type_block = self.getTypeBlock()
        if type_block:
            return type_block.parameters.get(param)

    def orderedParameters(self):
        """
        Utility function to get a list of ParameterInfos in
        the same order as in self.parameters_list
        Return:
            list[ParameterInfo]
        """
        ps = []
        for p in self.parameters_list:
            ps.append(self.parameters[p])
        return ps

    def getTypeBlock(self):
        """
        Gets the block for the current type.
        Return:
            BlockInfo if found else None
        """
        if self.types and self.parameters.get("type"):
            type_info = self.parameters["type"]
            if type_info.value and type_info.value in self.types:
                return self.types[type_info.value]

    def paramValue(self, param):
        """
        Gets the value of a parameter.
        Input:
            param[str]: Name of the parameter
        Return:
            str: Value of the parameter or None
        """
        param_info = self.getParamInfo(param)
        if param_info:
            return param_info.value

    def setParamValue(self, param, val):
        """
        Sets the value of a parameter.
        Input:
            param[str]: Name of the parameter
            val[str]: New value of the parameter
        """
        param_info = self.getParamInfo(param)
        if param_info:
            param_info.value = val

    def blockType(self):
        """
        Gets the current type for this block.
        Return:
            block type [str] if found else None
        """
        pinfo = self.parameters.get("type")
        if pinfo:
            return pinfo.value

    def setBlockType(self, type_name):
        """
        Set the type for this block.
        Input:
            type_name[str]: New type name
        """
        pinfo = self.parameters.get("type")
        if pinfo:
            pinfo.value = type_name

    def addChildBlock(self, child_info):
        """
        Add a new child to this block.
        Input:
            child_info[BlockInfo]: Child to be added
        """
        child_info.parent = self
        self.children[child_info.name] = child_info
        self.children_list.append(child_info.name)
        child_info.updatePaths()

    def updatePaths(self):
        """
        Make sure this node and all of its children have the correct paths
        """
        self.path = os.path.join(self.parent.path, self.name)
        for c in self.children.values():
            c.updatePaths()

    def removeChildBlock(self, name):
        """
        Remove a child.
        Input:
            name[str]: Name of the child to remove
        Return:
            BlockInfo of the removed child
        """
        child = self.children.get(name)
        if child:
            del self.children[name]
            self.children_list.remove(name)
            child.parent = None
            return child

    def renameChildBlock(self, oldname, newname):
        """
        Rename one of the children
        Input:
            oldname[str]: Current name of the child
            newname[str]: New name of the child
        """
        tmp_child = self.children.get(newname)
        if tmp_child:
            mooseutils.mooseWarning("Tried to rename %s to %s but %s already exists." % (oldname, newname, newname))
            return

        child = self.children.get(oldname)
        if child:
            del self.children[oldname]
            self.children[newname] = child
            idx = self.children_list.index(oldname)
            self.children_list.remove(oldname)
            self.children_list.insert(idx, newname)
            child.name = newname
            child.updatePaths()

    def addParameter(self, param):
        """
        Adds a parameter.
        Input:
            param[ParameterInfo]: New parameter to be added
        """
        param.parent = self
        self.parameters[param.name] = param
        if param not in self.parameters_list:
            self.parameters_list.append(param.name)

    def addUserParam(self, param, value):
        """
        Adds a user parameter.
        Input:
            param[str]: Name of the parameter to add
            value[str]: Initial value of the parameter
        Return:
            ParameterInfo: The new parameter
        """
        pinfo = self.getParamInfo(param)
        if pinfo:
            mooseutils.mooseWarning("Tried to add a user parameter when that name already exists: %s:%s" % (self.path, param))
            return
        pinfo = ParameterInfo(self, param)
        pinfo.user_added = True
        pinfo.value = value
        self.addParameter(pinfo)
        return pinfo

    def removeUserParam(self, name, force=False):
        """
        Remove a user added parameter.
        Input:
            name[str]: Name of the parameter to remove.
        """
        pinfo = self.getParamInfo(name)
        if pinfo and (pinfo.user_added or force):
            del self.parameters[pinfo.name]
            self.parameters_list.remove(name)
            pinfo.parent = None

    def renameUserParam(self, oldname, newname):
        """
        Rename a user parameter.
        Input:
            oldname[str]: Current name of the parameter.
            newname[str]: New name of the parameter.
        """
        pinfo = self.getParamInfo(oldname)
        if pinfo and pinfo.user_added:
            del self.parameters[oldname]
            orig_index = self.parameters_list.index(oldname)
            self.parameters_list.insert(orig_index, newname)
            self.parameters_list.remove(oldname)
            pinfo.name = newname
            self.parameters[newname] = pinfo

    def moveUserParam(self, param, new_index):
        """
        Move a user parameter. This just changes the order in which it will be written out.
        Input:
            param[str]: Name of the parameter.
            new_index[int]: New index in the parameter list
        """
        pinfo = self.getParamInfo(param)
        if pinfo:
            self.parameters_list.remove(param)
            self.parameters_list.insert(new_index, param)

    def moveChildBlock(self, name, new_index):
        """
        Moves a child block. This just changes the order in which it will be written out.
        Input:
            name[str]: Name of the child block.
            new_index[int]: New index of the child.
        """
        cinfo = self.children.get(name)
        if cinfo:
            self.children_list.remove(name)
            self.children_list.insert(new_index, name)

    def copy(self, parent):
        """
        Makes a copy of this node.
        Makes a recursive copy of all children, types, star node, etc.
        Input:
            parent[BlockInfo]: Parent of the copied block.
        Return:
            BlockInfo: A copy of this block
        """
        new = copy.copy(self)
        new.parent = parent
        new.children = {}
        new.children_list = []
        new.children_write_first = []
        for key in self.children_list:
            c = self.children[key]
            new.children_list.append(c.name)
            new.children[key] = c.copy(new)

        if self.star_node:
            new.star_node = self.star_node.copy(new)

        new.types = {}
        for key, val in self.types.items():
            new.types[key] = val.copy(new)

        new.parameters = {}
        new.parameters_list = []
        new.parameters_write_first = []

        for key in self.parameters_list:
            p = self.parameters[key]
            new.parameters_list.append(p.name)
            new.parameters[p.name] = p.copy(new)
        return new

    def addBlockType(self, type_info):
        """
        Adds a new type block.
        Input:
            type_info[BlockInfo]: A new type block
        """
        self.types[type_info.name] = type_info
        type_info.parent = self

    def setStarInfo(self, star_info):
        """
        Sets the star node for this block
        Input:
            star_info[BlockInfo]: The star block
        """
        self.star_node = star_info
        self.star = True
        star_info.parent = self

    def toolTip(self):
        """
        A suitable description that could be used in a tool tip.
        Return:
            str: A description of this block.
        """
        return self.description

    def findFreeChildName(self):
        """
        Tries to find a free node name.
        Starts from 0 and looks for an existing child block with name base_name{number}
        Returns:
            str: freely available node name
        """
        for i in range(10000):
            name = "New_%s" % i
            if name not in self.children_list:
                return name

    def dump(self, indent=0, sep='  '):
        """
        Provides a description of this block with all of its children, types, etc.
        Input:
            indent[int]: Current indentation level
            sep[str]: The indent string
        Return:
            str: The dump of this block.
        """
        o = StringIO()
        i_str = sep*indent
        o.write("%sPath: %s\n" % (i_str, self.path))
        indent += 1
        if self.parent:
            o.write("%sParent: %s\n" % (i_str, self.parent.path))
        o.write("%sChildren: %s\n" % (i_str, self.children_list))
        o.write("%sTypes: %s\n" % (i_str, self.types.keys()))
        o.write("%sHard: %s\n" % (i_str, self.hard))
        o.write("%sUser: %s\n" % (i_str, self.user_added))
        o.write("%sStar: %s\n" % (i_str, self.star))
        o.write("%sIncluded: %s\n" % (i_str, self.included))
        o.write("%sDescription: %s\n" % (i_str, self.description))
        o.write("%sParameters:\n" % i_str)
        for p in self.parameters.values():
            o.write("%s%s:\n" % ((indent+1)*sep, p.name))
            p.dump(o, indent+2, sep)

        o.write("%sChildren:\n" % (indent*sep))
        for name in self.children_list:
            c = self.children[name]
            o.write(c.dump(indent+1, sep))
        o.write("%sStar node:\n" % (indent*sep))
        if self.star_node:
            o.write(self.star_node.dump(indent+1, sep))
        o.write("%sType nodes:\n" % (indent*sep))
        for t in self.types.values():
            o.write(t.dump(indent+1, sep))
        return o.getvalue()

    def getParamNames(self):
        """
        Get the parameter names in the required order.
        Parameter names specified in the input file are printed
        out in the same order as in the original input file,
        followed by any other parameters that were changed.
        Return:
            list[str]: List of parameter names
        """
        return self._orderedNames(self.parameters_write_first, self.parameters_list)

    def getChildNames(self):
        """
        Get the child names in the required order.
        Child names specified in the input file are printed
        out in the same order as in the original input file,
        followed by any other children that were changed.
        Return:
            list[str]: List of child names
        """
        return self._orderedNames(self.children_write_first, self.children_list)

    def _orderedNames(self, first, complete):
        """
        Add in elements from the list "complete" to the end
        of the "first" if they are not already in "first"
        Input:
            first[list]: These elements will be first in the returned list
            complete[list]: These elements will come after first
        Return:
            list: The elements in "complete" with elements in "first" first.
        """
        l = first[:]
        for x in complete:
            if x not in l:
                l.append(x)
        return l
