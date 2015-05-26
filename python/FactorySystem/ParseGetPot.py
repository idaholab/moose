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

  ##
  # Build a string suitable for writing to a raw input file
  # @param level The indentation level to apply to the string
  def createString(self, level = 0):

    # String to be returned
    output = ''

    # Write the block headings
    if level == 0:
       output += '[' + self.name + ']\n'
    elif level > 0:
      output += ' '*2*level + '[./' + self.name + ']\n'

    # Write the parameters
    for param in self.params_list:
      output += ' '*2*(level + 1) + param + " = '" + str(self.params[param]) + "'\n"

    # Write the children
    for child in self.children_list:
      output += self.children[child].createString(level + 1) + '\n'

    # Write the block closing
    if level == 0:
      output += '[]\n'
    elif level > 0:
      output += ' '*2*level + '[../]'

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

    self.parameter_res = [re.compile(r"\s*([\w\-]+)\s*=\s*'([^\n]+)'"),  # parameter value in two single ticks
                          re.compile(r'\s*([\w\-]+)\s*=\s*"([^\n]+)"'),  # parameter value in two double ticks
                          re.compile(r"\s*([\w\-]+)\s*=\s*'([^'\n]+)"),   # parameter value with a single single tick
                          re.compile(r'\s*([\w\-]+)\s*=\s*"([^"\n]+)'),   # parameter value with a single double tick
                          re.compile(r"\s*(\w+)\s*=\s*([^#'""\n\[\]\s]+)")] # parameter value with no double/single tick

    self.comment_re = re.compile(r"\s*(?:'.*')?\s*#\s*(.*)")

    self.unmatched_ticks_re = [[re.compile(r"[^']*'[^']*\n"), re.compile(r"\s*([^'\n]+)"), "'"],   # unmatched single tick and appropriate data re
                               [re.compile(r'[^"]*"[^"]*\n'), re.compile(r'\s*([^"\n]+)'), '"']]   # unmatched double tick and appropriate data re

    self._parseFile()

  def _recursiveParseFile(self, current_node, lines, current_line, current_position):
    param_name = '' # We need to store the name of the last parameter that has been identified to
                    # properly assign comments. If param_name=='', we assign them to the section.
    while current_line < len(lines):
      if current_position >= len(lines[current_line]) or current_position == -1:
        # reached end of current line
        current_line += 1
        current_position = 0
        param_name = '' # comments that are not preceded by any parameter belong to the section

      if current_line == len(lines):
        # file traversal finished
        return len(lines), len(lines[-1])

      # we are only interested in any part of the line that has not been parsed yet
      line = lines[current_line][current_position:]
      #print current_line, current_position, 'of', len(lines[current_line]), line

      m = self.section_begin_re.match(line)
      if m:
        current_position += m.end()
        child_name = m.group(2)

        if child_name in current_node.children:
          child = current_node.children[child_name]
        else:
          child = GPNode(child_name, current_node)
          current_node.children[child_name] = child
          current_node.children_list.append(child_name)

        current_line, current_position = self._recursiveParseFile(child, lines, current_line, current_position)
        continue

      # Look for a parameter on this line
      for re_param in self.parameter_res:
        m = re_param.match(line)

        if m:
          current_position += m.end()
          param_name = m.group(1)
          param_value = m.group(2)

          # See if the value of this parameter has an unmatched single tick
          for re_tick in self.unmatched_ticks_re:
            # Only look at the part before the comment (if there is one)
            m_tick = re_tick[0].match(line.partition('#')[0])
            if m_tick:
              current_line += 1
              current_position = 0
              found_it = False
              # in case of a multiline parameter, we have to remove the leading single/double tick
              param_value = param_value.lstrip(re_tick[2])
              # Keep eating lines until we find its mate
              while current_line < len(lines):
                line = lines[current_line]

                # While we're eating lines keep appending data to the value for this parameter
                m_data = re_tick[1].match(line)
                if m_data:
                  param_value += ' ' + m_data.group(1)

                m_tick = re_tick[0].match(line.partition('#')[0]) # Don't include the comment
                if m_tick:
                  found_it = True
                  break

                current_line += 1
              if not found_it:
                raise ParseException("SyntaxError", "Unmatched token in Parser")

              break # do not continue searching for unmatched ticks

          unique_key = current_node.fullName(True) + '/' + param_name
          if unique_key in self.unique_keys:
            raise ParseException("DuplicateSymbol", 'In file: ' + os.getcwd() + '/' + os.path.basename(self.file_name) + " \n Duplicate section name: " + unique_key)
          self.unique_keys.add(unique_key)

          current_node.params[param_name] = param_value
          current_node.params_list.append(param_name)

          break

      if m:
        continue # with outer loop since we found a parameter and have to remove it from the current line before continuing

      # Comment in the block (not after a parameter or section header)
      m = self.comment_re.match(line)
      if m:
        current_position = -1 # remainder of line ignored
        if param_name=='':
          current_node.comments.append(m.group(1))
        else:
          current_node.param_comments[param_name] = m.group(1)
        continue

      # Is this section over?
      m = self.section_end_re.match(line)
      if m:
        current_position += m.end()
        return current_line, current_position

      # did not find anything else in this line
      current_position = -1

    return current_line, current_position

  def _parseFile(self):
    lines = self.file.readlines()
    self._recursiveParseFile(self.root_node, lines, 0, 0)

def readInputFile(file_name):
  pgp = ParseGetPot(file_name)
#  pgp.root_node.Print()
  return pgp.root_node


if __name__ == '__main__':
  if (len(sys.argv) > 1):
    filename = sys.argv[1]
  else:
    filename = '2d_diffusion_test.i'

  pgp = ParseGetPot(filename)
  print 'Printing tree'
  pgp.root_node.Print()
