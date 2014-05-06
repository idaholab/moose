#!/usr/bin/python

import sys

sys.path.append('.')
import ParseGetPot 


class Parser:
  def __init__(self):
    return

  def parse(self, filename):

    try:
      root = ParseGetPot.readInputFile(filename)
    except:
      print "Parse Error: " + filename
      sys.exit(1)

    self._parseNode(root)

  def _parseNode(self, node):
    # Loop over the section names
    for child in node.children_list:
      print node.children[child].fullName()
      self._parseNode(node.children[child])


class Syntax:
  def __init__(self):
    self. 
    return

  def registerSyntax():
    


if __name__ == '__main__':
  parser = Parser()
  parser.parse('tests')
