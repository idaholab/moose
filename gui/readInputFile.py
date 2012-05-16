from GetPot import GetPot

"""
  Helper class for building a tree structure from the input file
  Each GPNode represents a section
"""
class GPNode:
  def __init__(self, name, parent):
    self.name = name
    self.parent = parent
    self.params = {}
    self.params_list = [] #This is here to capture the ordering
    self.children = {}
    self.children_list = []  #This is here to capture the ordering
  """ Print this node and it's children """
  def Print(self, prefix=''):
    print prefix + self.name
    
    for param in self.params_list:
      print prefix + param + ": " + str(self.params[param])
      
    for child in self.children_list:
      self.children[child].Print(prefix + self.name + '/')

""" Add all of the sections to the tree """
def buildSectionTree(ifpot, root_node):
  for section in ifpot.get_section_names():
    if section != '':
      split = section.split('/')
      current_node = root_node
      for subsection in split:
        if subsection != '':
          if not subsection in current_node.children: # New section node
            new_node = GPNode(subsection, current_node)
            current_node.children[subsection] = new_node
            current_node.children_list.append(subsection)
            current_node = new_node
          else:
            current_node = current_node.children[subsection]

""" Add all of the parameters to each section """
def populateParams(ifpot, root_node):
  for param in ifpot.get_variable_names():
    if param != '':
      split = param.split('/')
      current_node = root_node
      for subsection in split:
        if subsection != '':
          if not subsection in current_node.children: # Must be a parameter
              current_node.params[subsection] = ifpot(param,'')
              current_node.params_list.append(subsection)
          else:
            current_node = current_node.children[subsection]

"""Reads file_name and returns a dictionary of GPNodes that are the 'main blocks'"""
def readInputFile(file_name):
  ifpot = GetPot(Filename=file_name)
  root_node = GPNode('root', None)
  buildSectionTree(ifpot, root_node)
  populateParams(ifpot, root_node)
#  root_node.Print()
  return root_node

if __name__ == '__main__':
  readInputFile()
