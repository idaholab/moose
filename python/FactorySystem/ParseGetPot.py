#!/usr/bin/python

import sys, os, re

class GPNode:
  def __init__(self, name, parent):
    self.name = name
    self.parent = parent
    self.params = {}
    self.params_list = [] #This is here to capture the ordering
    self.param_comments = {}
    self.children = {}
    self.children_list = []  #This is here to capture the ordering
    self.comments = []

  """ Print this node and it's children """
  def Print(self, prefix=''):
    print prefix + self.name

    for comment in self.comments:
      print '# ' + comment

    for param in self.params_list:
      comment = ''
      if param in self.param_comments:
        comment = '# ' + self.param_comments[param]
      print prefix + self.name + '/' + param + ": " + str(self.params[param]) + ' | ' + comment


    for child in self.children_list:
      self.children[child].Print(prefix + self.name + '/')


  ##
  # Perform a fuzzy search for a node name
  # @return The node object if any part of a node key is in the supplied name
  def getNode(self, name):
    node = None
    if name in self.children:
      node = self.children[name]


    else:
      for key, value in self.children.iteritems():
        node = value.getNode(name)
        if node != None:
          break
    return node

  def fullName(self, no_root=False):
    if self.parent == None:
      if no_root and self.name == 'root':
        return ''
      else:
        return self.name
    else:
      return self.parent.fullName(no_root) + '/' + self.name

  def write(self, level = 0):

    # List to be returned
    output = []

    # Write the block headings
    if level == 0:
       output.append('[' + self.name + ']')
    elif level > 0:
      output.append(' '*2*level + '[./' + self.name + ']')

    # Write the parameters
    for param in self.params_list:
      output.append(' '*2*(level + 1) + param + " = '" + str(self.params[param] + "'"))

    # Write the children
    for child in self.children_list:
      output += self.children[child].write(level + 1)

    # Write the block closing
    if level == 0:
      output.append('[]\n')
    elif level > 0:
      output.append(' '*2*level + '[../]')

    # Return the data
    return output


class ParseException(Exception):
  def __init__(self, expr, msg):
    self.expr = expr
    self.msg = msg

class ParseGetPot:
  def __init__(self, file_name):
    self.file_name = file_name
    self.file = open(file_name)
    self.unique_keys = set() #The full path to each key to ensure that no duplicates are supplied

    self.root_node = GPNode('root', None)

    self.section_begin_re = re.compile(r"\s*\[\s*(\./)?([^(\.\./) \t\n\r\f\v]+)\s*]")

    self.section_end_re = re.compile(r"\s*\[\s*(\.\./)?\s*\]")

    self.parameter_re = re.compile(r"\s*(\w+)\s*=\s*([^#\n]+?)\s*(#.*)?\n")

    self.parameter_in_single_quotes_re = re.compile(r"\s*([\w\-]+)\s*=\s*'([^\n]+?)'\s*(#.*)?\n")

    self.comment_re = re.compile(r"[^']*(?:'.*')?\s*#\s*(.*)")

    self.unmatched_single_tick_re = re.compile(r"[^']*'[^']*\n")

    self.independent_data_re = re.compile(r"\s*([^'\n]+)")

    self._parseFile()

  def _recursiveParseFile(self, current_node, lines, current_position):
    while current_position < len(lines):
      if current_position == None:
        return len(lines)
      line = lines[current_position]
      m = self.section_begin_re.match(line)
      if m:
        child_name = m.group(2)

        if child_name in current_node.children:
          child = current_node.children[child_name]
        else:
          child = GPNode(child_name, current_node)
          current_node.children[child_name] = child
          current_node.children_list.append(child_name)

        # See if there are any comments after the beginning of the block
        m = self.comment_re.match(line)
        if m:
          child.comments.append(m.group(1))

        current_position += 1
        current_position = self._recursiveParseFile(child, lines, current_position)

        continue

      # Look for a parameter on this line
      m = self.parameter_in_single_quotes_re.match(line)

      if not m:
        m = self.parameter_re.match(line)

      if m:
        param_name = m.group(1)
        param_value = m.group(2)

        # See if the value of this parameter has an unmatched single tick
        # Only look at the part before the comment (if there is one)
        m = self.unmatched_single_tick_re.match(line.partition('#')[0])
        if m:
          current_position += 1
          found_it = False
          # Keep eating lines until we find its mate
          while current_position < len(lines):
            line = lines[current_position]

            # While we're eating lines keep appending data to the value for this parameter
            m = self.independent_data_re.match(line)
            if m:
              param_value += ' ' + m.group(1)

            m = self.unmatched_single_tick_re.match(line.partition('#')[0]) # Don't include the comment
            if m:
              found_it = True
              break

            current_position += 1
          if not found_it:
            raise ParseException("SyntaxError", "Unmatched token in Parser")

        unique_key = current_node.fullName(True) + '/' + param_name
        if unique_key in self.unique_keys:
          raise ParseException("DuplicateSymbol", 'Duplicate Section Name "' + os.getcwd() + '/' + self.file_name + ":" + unique_key + '"')
        self.unique_keys.add(unique_key)

        current_node.params[param_name] = param_value.strip("'")
        current_node.params_list.append(param_name)

        # See if this parameter has a comment after it
        m = self.comment_re.match(line)
        if m:
          current_node.param_comments[param_name] = m.group(1)

        current_position += 1

        continue

      # Comment in the block (not after a parameter)
      m = self.comment_re.match(line)
      if m:
        current_node.comments.append(m.group(1))

      # Is this section over?
      m = self.section_end_re.match(line)
      if m:
        current_position += 1
        return current_position

      current_position += 1

  def _parseFile(self):
    lines = self.file.readlines()
    self._recursiveParseFile(self.root_node, lines, 0)

def readInputFile(file_name):
  pgp = ParseGetPot(file_name)
#  pgp.root_node.Print()
  return pgp.root_node


if __name__ == '__main__':
  pgp = ParseGetPot('2d_diffusion_test.i')
#  pgp = ParseGetPot('input.i')
  print 'Printing tree'
  pgp.root_node.Print()
