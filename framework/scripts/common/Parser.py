#!/usr/bin/python

import sys

sys.path.append('.')
import ParseGetPot, Factory


class Parser:
  def __init__(self, factory):
    self.factory = factory

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
    self.associated_objects = {}
    return

  def registerSyntax(self, ):
    return  


if __name__ == '__main__':
  factory = Factory.Factory()

  # For now we'll just use the Testers
  factory.loadPlugins('../..', '/scripts/TestHarness/testers', Tester

  parser = Parser(factory)
  parser.parse('tests')

  
