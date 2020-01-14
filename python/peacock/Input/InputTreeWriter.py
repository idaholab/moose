#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from pyhit import hit

def addInactive(hit_parent, parent, children):
    """
    If there are any inactive blocks, add them
    """
    inactive = []
    for child in children:
        entry = parent.children.get(child, None)
        if entry and entry.checkInactive():
            inactive.append(entry.name)
    if inactive:
        hit_parent.addChild(hit.NewField("inactive", "String", "'%s'" % ' '.join(inactive)))

def inputTreeToString(root):
    """
    Main access point to write an InputTree to a string.
    Input:
        root[BlockInfo]: Root item of the tree.
    Return:
        str: The input file
    """
    children = root.getChildNames()
    hit_node = hit.NewSection("")
    if root.comments:
        commentNode(hit_node, root.comments, False)
    addInactive(hit_node, root, children)
    last_child = children[-1]
    for child in children:
        entry = root.children.get(child, None)
        if entry and entry.wantsToSave():
            addNode(hit_node, entry)
            if child != last_child:
                hit_node.addChild(hit.NewBlank())

    return hit_node.render()

def addNode(parent_hit_node, entry):
    """
    Adds a node and its children to the HIT tree.
    Input:
        parent_hit_node[hit.Node]: Parent to add children to
        entry[BlockInfo]: The block to add
    """
    name = entry.name
    hit_node = hit.NewSection(name)
    parent_hit_node.addChild(hit_node)
    if entry.comments:
        commentNode(hit_node, entry.comments, False)
    nodeParamsString(hit_node, entry)
    children = entry.getChildNames()
    addInactive(hit_node, entry, children)
    for child in children:
        child_entry = entry.children.get(child, None)
        if child_entry and child_entry.wantsToSave():
            addNode(hit_node, child_entry)

def nodeParamsString(hit_parent, entry, ignore_type=False):
    params = entry.getParamNames()
    for name in params:
        if ignore_type and name == "type":
            continue
        info = entry.parameters[name]
        val = info.inputFileValue()
        if not val and name != 'active': # we generally don't want to write out empty strings
            continue
        comments = info.comments
        if info.hasChanged() or info.user_added or info.set_in_input_file:
            hit_param = hit.NewField(info.name, info.hitType(), val)
            hit_parent.addChild(hit_param)
            if comments:
                commentNode(hit_param, comments, True)

    type_info = entry.parameters.get("type")
    if entry.types and type_info:
        type_name = type_info.value
        type_entry = entry.types.get(type_name)
        if type_entry:
            nodeParamsString(hit_parent, type_entry, ignore_type=True)

def commentNode(hit_parent, comments, is_inline):
    c_list = comments
    if isinstance(comments, str):
        c_list = [comments]
    for c in c_list:
        lines = c.split("\n")
        for line in lines:
            if line:
                line = "# %s" % line
            else:
                line = "#"
            hit_comment = hit.NewComment(line, is_inline)
            hit_parent.addChild(hit_comment)
