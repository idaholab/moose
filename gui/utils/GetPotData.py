#!/usr/bin/python
import sys, os, commands, time, re, copy

class GetPotData():
  def __init__(self, root_node, input_file_widget):
    self.input_file_widget = input_file_widget
    self.root_node = root_node

  def recursiveGetGPNode(self, current_node, pieces):
    if len(pieces) == 1 and pieces[0] == current_node.name:
      return current_node

    if pieces[1] in current_node.children:
      return self.recursiveGetGPNode(current_node.children[pieces[1]], pieces[1:])

    # See if there is a * here (this would be from an input template)
    if len(pieces) == 2 and '*' in current_node.children:
      return current_node.children['*']

    return None

  """ Returns the GetPot Node associated with the item... or None if there wasn't one """
  def getGPNode(self, item):
    if not self.root_node:
      return None
    else:
      path = self.input_file_widget.tree_widget.generatePathFromItem(item)
      pieces = path.split('/')
      return self.recursiveGetGPNode(self.root_node, pieces)
