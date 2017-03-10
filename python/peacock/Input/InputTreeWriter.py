import cStringIO

def inputTreeToString(root, sep="  "):
    """
    Main access point to write an InputTree to a string.
    Input:
        root[BlockInfo]: Root item of the tree.
    Return:
        str: The input file
    """
    output = cStringIO.StringIO()
    if root.comments:
        commentString(output, root.comments, 0, sep)
    last_child = root.children_list[-1]
    for child in root.children_list:
        entry = root.children[child]
        if entry.included:
            writeToString(output, entry, 0, sep, child == last_child)
    return output.getvalue()

def writeToString(output, entry, indent, sep="  ", is_last=False):
    """
    Write out this node and its children(recursively) to input file format.
    Input:
        curr_str[str]: Current input file
        indent[int]: indent level
    Return:
        str: input file
    """
    nodeHeaderString(output, entry, indent, sep)
    nodeParamsString(output, entry, indent+1, sep)
    for child in entry.children_list:
        child_entry = entry.children[child]
        if child_entry.included:
            writeToString(output, child_entry, indent+1, sep)
    nodeCloseString(output, entry, indent, sep, is_last)

def nodeHeaderString(output, entry, indent, sep):
    """
    While writing the input file, start a new section.
    Input:
        curr_str[str]: Current input file
        indent[int]: Current indent level
    Return:
        str: input file
    """
    str_indent = sep*indent
    if entry.parent.path != '/':
        output.write("%s[./%s]\n" % (str_indent, entry.name))
    else:
        output.write("[%s]\n" % entry.name)
    if entry.comments:
        commentString(output, entry.comments, indent+1, sep)

def nodeCloseString(output, entry, indent, sep, is_last):
    """
    While writing the input file, close a section
    Input:
        curr_str[str]: Current input file
        indent[int]: Current indent level
    Return:
        str: input file
    """
    if entry.parent.path != '/':
        output.write("%s[../]\n" % (sep*indent))
    elif is_last:
        output.write("[]\n")
    else:
        output.write("[]\n\n")

def nodeParamsString(output, entry, indent, sep, ignore_type=False):
    for name in entry.parameters_list:
        if ignore_type and name == "type":
            continue
        info = entry.parameters[name]
        val = info.inputFileValue()
        comments = info.comments
        if info.value != info.default or info.user_added or info.set_in_input_file:
            paramToString(output, name, val, comments, indent, sep)
    type_info = entry.parameters.get("type")
    if entry.types and type_info:
        type_name = type_info.value
        type_entry = entry.types.get(type_name)
        if type_entry:
            nodeParamsString(output, type_entry, indent, sep, ignore_type=True)

def commentString(output, comments, indent, sep):
    c_list = comments
    if isinstance(comments, str):
        c_list = [comments]
    for c in c_list:
        lines = c.split("\n")
        for line in lines:
          if not line:
            output.write("%s#\n" % (sep*indent))
          else:
            output.write("%s# %s\n" % (sep*indent, line))

def paramToString(output, name, val, comments, indent, sep):
    output.write("%s%s = %s" % (sep*indent, name, val))

    if comments:
        commentString(output, comments, indent, sep)
    else:
        output.write("\n")
