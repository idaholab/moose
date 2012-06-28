#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *
from ParamTable import *

import MeshInfo

from readInputFile import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileTreeWidget(QtGui.QTreeWidget):
  def __init__(self, input_file_widget, win_parent=None):
    QtGui.QTreeWidget.__init__(self, win_parent)

    self.input_file_widget = input_file_widget
    self.action_syntax = self.input_file_widget.action_syntax

    self.setExpandsOnDoubleClick(False)
    self.setMaximumWidth(300)
    self.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
    self.connect(self,QtCore.SIGNAL('customContextMenuRequested(QPoint)'), self._newContext)
    self.addHardPathsToTree()
      
    self.header().close()

    QtCore.QObject.connect(self,
                           QtCore.SIGNAL("itemDoubleClicked(QTreeWidgetItem *, int)"),
                           self._doubleClickedItem)
    
    QtCore.QObject.connect(self,
                           QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"),
                           self._itemChanged)
    
  def addHardPathsToTree(self):
    # Add every hard path
    for path in self.action_syntax.hard_paths:
      self._recursivelyAddTreeItems(path.split('/'), self)

  def loadData(self, counter, progress, main_sections):
    QtCore.QObject.disconnect(self, QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"), self._itemChanged)

    progress.setMaximum(counter+len(main_sections))

    for section_name, section_node in main_sections.items():
      counter+=1
      progress.setValue(counter)
      self._addDataRecursively(self, section_node)

    self.addHardPathsToTree() # We do this here because * paths might add more paths underneath some of the paths
    self.input_file_widget.input_file_textbox.updateTextBox()
    QtCore.QObject.connect(self, QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"), self._itemChanged)

  def generatePathFromItem(self, item):
    from_parent = ''
    if item.parent():
      from_parent = self.generatePathFromItem(item.parent())
      
    return from_parent + '/' + str(item.text(0))

  ''' Looks for a child item of parent named "name"... with return None if there is no child named that '''
  def findChildItemWithName(self, parent, name):
    try: # This will fail when we're dealing with the QTreeWidget itself
      num_children = parent.childCount()
    except:
      num_children = parent.topLevelItemCount()

    for i in range(num_children):
      child = None
      try: # This will fail when we're dealing with the QTreeWidget itself
        child = parent.child(i)
      except:
        child = parent.topLevelItem(i)
        
      if child.text(0) == name:
        return child
      
    return None

  def getMeshItemData(self):
    mesh_item = self.findChildItemWithName(self, 'Mesh')
    data = None
    try:
      return mesh_item.table_data
    except:
      pass
    
    return None

  def getMeshFileName(self):
    mesh_data = self.getMeshItemData()
    if mesh_data:
      if 'file' in mesh_data:
        return data['file']
    else:
      return None
    
  def _itemHasEditableParameters(self, item):
    this_path = self.generatePathFromItem(item)
    this_path = '/' + self.action_syntax.getPath(this_path) # Get the real action path associated with this item
    yaml_entry = self.input_file_widget.yaml_data.findYamlEntry(this_path)
    has_type_subblock = False
    if 'subblocks' in yaml_entry and yaml_entry['subblocks']:
      for sb in yaml_entry['subblocks']:
        if '<type>' in sb['name']:
          has_type_subblock = True

    if ('parameters' in yaml_entry and yaml_entry['parameters'] != None) or has_type_subblock or this_path == '/GlobalParams':
      return True

  def _addDataRecursively(self, parent_item, node):
    is_active = 'active' not in node.parent.params or node.parent.params['active'].find(node.name) != -1
    table_data = node.params
    table_data['Name'] = node.name

    new_child = self.findChildItemWithName(parent_item, table_data['Name'])

    if not new_child:  # If we didn't find a child that already matched then create a new child
      new_child = QtGui.QTreeWidgetItem(parent_item)
      new_child.setText(0,table_data['Name'])
      parent_item.addChild(new_child)
      new_child.table_data = {}

    has_params = False
    # See if there are any actual parameters for this item
    for name,value in node.params.items():
      if name != 'active':
        has_params = True

    if has_params:
      new_child.table_data = table_data
    new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)

    if is_active:
      new_child.setCheckState(0, QtCore.Qt.Checked)
    else:
      new_child.setCheckState(0, QtCore.Qt.Unchecked)

    for child, child_node in node.children.items():
      self._addDataRecursively(new_child, child_node)      

  def _recursivelyAddTreeItems(self, split_path, parent):
    this_piece = split_path[0]

    this_item = None
    found_it = False
    is_star = False

    if this_piece == '*':
      found_it = True
      is_star = True

    num_children = 0

    try: # This will fail when we're dealing with the QTreeWidget itself
      num_children = parent.childCount()
    except:
      num_children = parent.topLevelItemCount()

    for i in range(num_children):
      child = None
      try: # This will fail when we're dealing with the QTreeWidget itself
        child = parent.child(i)
      except:
        child = parent.topLevelItem(i)
        
      if child.text(0) == this_piece:
        this_item = child
        found_it = True

    if not found_it:
      # Add it
      this_item = QtGui.QTreeWidgetItem(parent)
      this_item.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      this_item.setCheckState(0, QtCore.Qt.Unchecked)
      this_item.setText(0, this_piece)
      this_path = self.generatePathFromItem(this_item)
      if self.action_syntax.hasStar(this_path):
        this_item.setForeground(0, Qt.blue)

    if len(split_path) > 1:
      if not is_star:
        self._recursivelyAddTreeItems(split_path[1:], this_item)
      else: # If it is a star and there are children - then add it to all of the children
        for i in range(num_children):
          child = None
          try: # This will fail when we're dealing with the QTreeWidget itself
            child = parent.child(i)
          except:
            child = parent.topLevelItem(i)
          self._recursivelyAddTreeItems(split_path[1:], child)

  def _getChildNames(self, parent):
    try: # This will fail when we're dealing with the QTreeWidget itself
      num_children = parent.childCount()
    except:
      num_children = parent.topLevelItemCount()

    children_names = []

    for i in range(num_children):
      child = None
      try: # This will fail when we're dealing with the QTreeWidget itself
        child = parent.child(i)
      except:
        child = parent.topLevelItem(i)
        
      children_names.append(child.text(0))
      
    return children_names

  def _typeOptions(self):
    type_options = {}

    # Variables
    variables_item = self.input_file_widget.tree_widget.findItems("Variables", QtCore.Qt.MatchExactly)[0]
    variable_names = self._getChildNames(variables_item)
    if len(variable_names):
      type_options['std::vector<VariableName>'] = variable_names
      type_options['VariableName'] = variable_names

    aux_variables_item = self.input_file_widget.tree_widget.findItems("AuxVariables", QtCore.Qt.MatchExactly)[0]
    aux_variable_names = self._getChildNames(aux_variables_item)
    if len(aux_variable_names):
      type_options['std::vector<VariableName>'] += aux_variable_names
      type_options['VariableName'] += aux_variable_names

    functions_item = self.input_file_widget.tree_widget.findItems("Functions", QtCore.Qt.MatchExactly)[0]
    function_names = self._getChildNames(functions_item)
    if len(function_names):
      type_options['std::vector<FunctionName>'] = function_names
      type_options['FunctionName'] = function_names

    # Mesh stuff
    mesh_data = self.getMeshItemData()
    if mesh_data:
      mesh_info = MeshInfo.getMeshInfo(mesh_data)

      if mesh_info:
        type_options['BlockName'] = mesh_info.blockNames()
        type_options['std::vector<BlockName>'] = mesh_info.blockNames()
        type_options['BoundaryName'] = mesh_info.sidesetNames()
        type_options['std::vector<BoundaryName>'] = mesh_info.sidesetNames()
      
    return type_options
    
  def _doubleClickedItem(self, item, column):
    this_path = self.generatePathFromItem(item)

    if not self.action_syntax.isPath(this_path) or self._itemHasEditableParameters(item):
      already_had_data = False
      try:
        item.table_data # If this fails we will jump to "except"...
        already_had_data = True
      except:
        item.table_data = None

      parent_path = ''

      if self.action_syntax.isPath(this_path):
        this_path = '/' + self.action_syntax.getPath(this_path) # Get the real action path associated with this item
        parent_path = this_path
      else:
        parent_path = self.generatePathFromItem(item.parent())
        parent_path = '/' + self.action_syntax.getPath(parent_path)
      yaml_entry = self.input_file_widget.yaml_data.findYamlEntry(parent_path)

      new_gui = OptionsGUI(yaml_entry, self.action_syntax, item.text(column), item.table_data, False, self._typeOptions())

      if item.table_data:
        new_gui.incoming_data = item.table_data

      if new_gui.exec_():
        item.table_data = new_gui.result()
        if not self.action_syntax.isPath(this_path):  # Don't change the name of hard paths
          item.setText(0,item.table_data['Name'])
        if not already_had_data:
          item.setCheckState(0, QtCore.Qt.Checked)
        self.input_file_widget.input_file_textbox.updateTextBox()

  def _itemChanged(self, item, column):
    self.input_file_widget.input_file_textbox.updateTextBox()

  def _deleteCurrentItem(self):
    item = self.currentItem()
    parent = item.parent()
    if parent:
      parent.removeChild(item)
    else: #Must be a top level item
      self.removeItemWidget(item, 0)
    self.addHardPathsToTree() # We do this here because they might have removed a hard path... but there is no way to get them back
    self.input_file_widget.input_file_textbox.updateTextBox()

  def _addItem(self):
    item = self.currentItem()
    this_path = self.generatePathFromItem(item)
    this_path = '/' + self.action_syntax.getPath(this_path) # Get the real action path associated with this item
    yaml_entry = self.input_file_widget.yaml_data.findYamlEntry(this_path)

    self.new_gui = OptionsGUI(yaml_entry, self.action_syntax, item.text(0), None, False, self._typeOptions())
    if self.new_gui.exec_():
      table_data = self.new_gui.result()
      new_child = QtGui.QTreeWidgetItem(item)
      new_child.setText(0,table_data['Name'])
      new_child.table_data = table_data
      new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      new_child.setCheckState(0, QtCore.Qt.Checked)
      item.addChild(new_child)
      item.setCheckState(0, QtCore.Qt.Checked)
      item.setExpanded(True)
      self.input_file_widget.updateTextBox()
      self.addHardPathsToTree() # We do this here because * paths might add more paths underneath the item we just added

  def _newContext(self, pos):
    global_pos = self.mapToGlobal(pos)
    item = self.itemAt(pos)
    this_path = self.generatePathFromItem(item)

    menu = QtGui.QMenu(self)
    
    # Don't allow deletion of hard paths
    if self.action_syntax.hasStar(this_path): # If it is a hard path allow them to add a child
      add_action = QtGui.QAction("Add...", self)
      add_action.triggered.connect(self._addItem)
      menu.addAction(add_action)
    else:
      delete_action = QtGui.QAction("Delete", self)
      delete_action.triggered.connect(self._deleteCurrentItem)
      menu.addAction(delete_action)
      
    menu.popup(global_pos)

