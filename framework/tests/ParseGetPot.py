#!/usr/bin/python

import sys, re

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
      print prefix + self.name + '/' + param + ": " + str(self.params[param]) + '|' + comment


    for child in self.children_list:
      self.children[child].Print(prefix + self.name + '/')

class ParseException(Exception):
  def __init__(self, expr, msg):
    self.expr = expr
    self.msg = msg

class ParseGetPot:
  def __init__(self, file_name):
    self.file_name = file_name
    self.file = open(file_name)

    self.root_node = GPNode('root', None)

    self.section_begin_re = re.compile(r"\s*\[\s*(\./)?([^(\.\./) \t\n\r\f\v]+)\s*]")

    self.section_end_re = re.compile(r"\s*\[\s*(\.\./)?\s*\]")

    self.parameter_re = re.compile(r"\s*(\w+)\s*=\s*([^#\n]+?)\s*(#.*)?\n")

    self.comment_re = re.compile(r"[^#]*#\s*(.*)")

    self.unmatched_single_tick_re = re.compile(r"[^']*'[^']*\n")
    self.independent_data_re = re.compile(r"\s*([^(#.*\n)]+)")

    self._parseFile()

  def _recursiveParseFile(self, current_node, lines, current_position):
    while current_position < len(lines):
      if current_position == None:
        return len(lines)
      line = lines[current_position]
      m = self.section_begin_re.match(line)
      if m:
        child_name = m.group(2)
        child = GPNode(child_name, current_node)

        # See if there are any comments after the beginning of the block
        m = self.comment_re.match(line)
        if m:
          child.comments.append(m.group(1))

        current_node.children[child_name] = child
        current_node.children_list.append(child_name)

        current_position += 1
        current_position = self._recursiveParseFile(child, lines, current_position)

        continue

      # Look for a parameter on this line
      m = self.parameter_re.match(line)
      if m:
        param_name = m.group(1)
        param_value = m.group(2)

        # See if the value of this parameter has an unmatched single tick
        m = self.unmatched_single_tick_re.match(line)
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

            m = self.unmatched_single_tick_re.match(line)
            if m:
              found_it = True
              break

            current_position += 1
          if not found_it:
            raise ParseException("TODO", "TODO")

        current_node.params[param_name] = param_value
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
  return pgp.root_node


if __name__ == '__main__':
  pgp = ParseGetPot('2d_diffusion_test.i')
#  pgp = ParseGetPot('input.i')
  print 'Printing tree'
  pgp.root_node.Print()
