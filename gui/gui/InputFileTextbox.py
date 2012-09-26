#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *
from ParamTable import *

from readInputFile import readInputFile, GPNode
from mergeLists import mergeLists

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileTextbox(QtGui.QTextEdit):
  def __init__(self, input_file_widget, win_parent=None):
    QtGui.QTextEdit.__init__(self, win_parent)
    self.input_file_widget = input_file_widget
#    self.setMinimumWidth(300)
    self.setReadOnly(True)
    self.textbox_layout = QtGui.QVBoxLayout()
    self.textbox_layout.addWidget(self)
    self.textbox_layout.setSizeConstraint(QtGui.QLayout.SetMinimumSize)

  def getLayout(self):
    return self.textbox_layout

  def buildInputString(self):
    self.the_string = ''
    root = self.input_file_widget.tree_widget.invisibleRootItem()
    child_count = root.childCount()
    printed_sections = []

    ordered_sections = []
    template_sections = []

    # Print out comments in the main section
    if self.input_file_widget.tree_widget.comment != '':
      split_comment = self.input_file_widget.tree_widget.comment.split('\n')
      for line in split_comment:
        self.the_string += '# ' + line + '\n'

      self.the_string += '\n'
    
    if self.input_file_widget.input_file_root_node:
      for section_name in self.input_file_widget.input_file_root_node.children_list:
        ordered_sections.append(section_name)

    if self.input_file_widget.input_file_template_root_node:
      for section_name in self.input_file_widget.input_file_template_root_node.children_list:
        template_sections.append(section_name)

    #Merge the sections
    mergeLists(ordered_sections, template_sections)

    for section_name in ordered_sections:
      search = self.input_file_widget.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)

      if search and len(search):
        item = search[0]
        self._inputStringRecurse(item, 0)
        printed_sections.append(section_name)          

    for i in range(child_count): # Print out all the other sections
      item = root.child(i)
      if item.text(0) not in printed_sections:
        self._inputStringRecurse(item, 0)
        
    return self.the_string

  def updateTextBox(self):
    # Save off the current position
    position = self.verticalScrollBar().sliderPosition()

    # Reset the text
    self.setText(self.buildInputString())

    # Scroll back to where we were
    self.verticalScrollBar().setValue(position)


  def _inputStringRecurse(self, item, level):    
    indent_string = ''
    for i in xrange(0,level):
      indent_string += '  '
      
    this_path = self.input_file_widget.tree_widget.generatePathFromItem(item)

    # Don't print hard paths that aren't checked
    if self.input_file_widget.action_syntax.isPath(this_path) and item.checkState(0) != QtCore.Qt.Checked:
      return
    
    else:
      section = item.text(0)
      subchild_count = item.childCount()
      gp_node = None
      template_gp_node = None

      if self.input_file_widget.input_file_root_node:
        gp_node = self.input_file_widget.input_file_getpot_data.getGPNode(item)

      if self.input_file_widget.input_file_template_root_node:
        template_gp_node = self.input_file_widget.input_file_template_getpot_data.getGPNode(item)

      if level == 0:
        self.the_string += '[' + section + ']\n'
      else:
        self.the_string += indent_string + '[./' + section + ']\n'

      try:
        # Print out comments for this block
        if item.comment != '':
          split_comment = item.comment.split('\n')
          for line in split_comment:
            self.the_string += indent_string + '  # ' + line + '\n'
      except:
        pass

      active = []
      has_inactive_children = False # Whether or not it has inactive children
      
      # Print the active line if necessary
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.checkState(0) == QtCore.Qt.Checked:
          active.append(str(subitem.text(0)))
        else:
          if not self.input_file_widget.action_syntax.isPath(self.input_file_widget.tree_widget.generatePathFromItem(subitem)) and str(subitem.text(0)) != 'ParentParams':
            has_inactive_children = True

      if has_inactive_children:
          self.the_string += indent_string + '  ' + "active = '" + ' '.join(active) + "'\n"

      # Print out the params for this item
      try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
        printed_params = []

        ordered_params = []
        template_params = []

        if gp_node:
          for param in gp_node.params_list:
            if param in item.table_data and param != 'Name' and param != 'parent_params':
              ordered_params.append(param)
              
        if template_gp_node:
          for param in template_gp_node.params_list:
            if param in item.table_data and param != 'Name' and param != 'parent_params':
              template_params.append(param)

        # Merge Lists
        mergeLists(ordered_params, template_params)

        # Print out the merged lists
        for param in ordered_params:
          comment = ''
          if param in item.param_comments:
            comment = ' # ' + item.param_comments[param]

          param_value = item.table_data[param]
          if ' ' in param_value:
            param_value = "'"+param_value.strip("'")+"'"

          self.the_string += indent_string + '  ' + param + ' = ' + param_value + comment + '\n'
          printed_params.append(param)
        
        for param,value in item.table_data.items():
          if param not in printed_params and param != 'Name' and param != 'parent_params':
            comment = ''
            if param in item.param_comments:
              comment = ' # ' + item.param_comments[param]

            param_value = value
            if ' ' in param_value:
              param_value = "'"+param_value.strip("'")+"'"
  
            self.the_string += indent_string + '  ' + param + ' = ' + param_value + comment + '\n'
      except:
        pass

      # Now recurse over the children
      printed_children = []

      ordered_children = []
      template_children = []

      if gp_node:
        for child in gp_node.children_list:
          ordered_children.append(child)

      if template_gp_node:
        for child in template_gp_node.children_list:
          template_children.append(child)

      mergeLists(ordered_children, template_children)
      
      for child in ordered_children:
        for j in range(subchild_count):
          subitem = item.child(j)

          if subitem.text(0) != child:
            continue

          self._inputStringRecurse(subitem, level+1)
          printed_children.append(child)
      
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.text(0) in printed_children:
          continue        

        self._inputStringRecurse(subitem, level+1)

    if level == 0:
      self.the_string += '[]\n\n'
    else:
      self.the_string += indent_string + '[../]\n'
