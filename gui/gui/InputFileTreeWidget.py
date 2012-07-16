#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *
from ParamTable import *
from CommentEditor import *

import MeshInfoFactory

from readInputFile import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileTreeWidget(QtGui.QTreeWidget):
  tree_changed = QtCore.pyqtSignal()
  mesh_item_changed = QtCore.pyqtSignal(QtGui.QTreeWidgetItem)
  
  def __init__(self, input_file_widget, win_parent=None):
    QtGui.QTreeWidget.__init__(self, win_parent)

    self.comment = ''

    self.input_file_widget = input_file_widget
    self.action_syntax = self.input_file_widget.action_syntax

    self.setExpandsOnDoubleClick(False)
    self.setMinimumWidth(200)
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

    QtCore.QObject.connect(self,
                           QtCore.SIGNAL("currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)"),
                           self._currentItemChanged)

    
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
    self._updateOtherGUIElements()
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
        return mesh_data['file']
    else:
      return None

  def getOutputItemData(self):
    output_item = self.findChildItemWithName(self, 'Output')
    data = None
    try:
      return output_item.table_data
    except:
      pass
    
    return None

  def getOutputFileNames(self):
    output_item = self.findChildItemWithName(self, 'Output')
    oversampling_item = self.findChildItemWithName(output_item, 'OverSampling')

    file_names = []     
    file_base = ''

    output_data = None

    if oversampling_item.checkState(0) == QtCore.Qt.Checked:
      output_data =  output_item.table_data

      if output_data:
        if 'file_base' in output_data:
          file_base = output_data['file_base'] + '_oversample'
        else:
          file_base = 'peacock_run_tmp_out_oversample'
      
    elif output_item.checkState(0) == QtCore.Qt.Checked:
      output_data =  output_item.table_data

      if output_data:
        if 'file_base' in output_data:
          file_base = output_data['file_base']
        else:
          file_base = 'peacock_run_tmp_out'

    type_to_extension = {'exodus':'.e', 'tecplot':'.plt'}

    # FIXME: Hack to make raven and r7 work for now
    if 'raven' in self.input_file_widget.app_path or 'r7' in self.input_file_widget.app_path:
      file_base += '_displaced'

    for atype,extension in type_to_extension.items():
      if output_data and atype in output_data and (output_data[atype] == 'true' or output_data[atype] == '1'):
        file_names.append(file_base + extension)

    return file_names
    
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

    param_comments = node.param_comments

    comment = '\n'.join(node.comments)

    new_child = self.findChildItemWithName(parent_item, table_data['Name'])

    if not new_child:  # If we didn't find a child that already matched then create a new child
      new_child = QtGui.QTreeWidgetItem(parent_item)
      new_child.setText(0,table_data['Name'])
#      parent_item.addChild(new_child)
      new_child.table_data = {}
      new_child.param_comments = []
      new_child.comment = ''
      

    has_params = False
    # See if there are any actual parameters for this item
    for name,value in node.params.items():
      if name != 'active':
        has_params = True

    if has_params:
      new_child.table_data = copy.deepcopy(table_data)
      if 'active' in new_child.table_data:
        del new_child.table_data['active']
      new_child.param_comments = param_comments
      
    new_child.comment = comment
    
    new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)

    if is_active:
      new_child.setCheckState(0, QtCore.Qt.Checked)
    else:
      new_child.setCheckState(0, QtCore.Qt.Unchecked)

    if new_child.text(0) == 'Mesh':
      if 'type' not in new_child.table_data:
        new_child.table_data['type'] = 'MooseMesh'
      self.mesh_item_changed.emit(new_child)

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
      this_item.table_data = {}
      this_item.param_comments = []
      this_item.comment = ''

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
    variables_items = self.input_file_widget.tree_widget.findItems("Variables", QtCore.Qt.MatchExactly)
    if variables_items:
      variables_item = variables_items[0]
      variable_names = self._getChildNames(variables_item)
      if len(variable_names):
        type_options['std::vector<NonlinearVariableName>'] = set(variable_names)
        type_options['NonlinearVariableName'] = set(variable_names)
        type_options['std::vector<VariableName>'] = set(variable_names)
        type_options['VariableName'] = set(variable_names)

    aux_variables_items = self.input_file_widget.tree_widget.findItems("AuxVariables", QtCore.Qt.MatchExactly)
    if aux_variables_items:
      aux_variables_item = aux_variables_items[0]
      aux_variable_names = self._getChildNames(aux_variables_item)
      if len(aux_variable_names):
        type_options['std::vector<AuxVariableName>'] = set(aux_variable_names)
        type_options['AuxVariableName'] = set(aux_variable_names)
        type_options['std::vector<VariableName>'] |= set(aux_variable_names)
        type_options['VariableName'] |= set(aux_variable_names)

    functions_items = self.input_file_widget.tree_widget.findItems("Functions", QtCore.Qt.MatchExactly)
    if functions_items:
      functions_item = functions_items[0]
      function_names = self._getChildNames(functions_item)
      if len(function_names):
        type_options['std::vector<FunctionName>'] = set(function_names)
        type_options['FunctionName'] = set(function_names)

    # Mesh stuff
    mesh_data = self.getMeshItemData()
    print 'Mesh Data:', mesh_data
    if mesh_data:
      mesh_info = MeshInfoFactory.getMeshInfo(mesh_data)
      print 'Mesh Info:', mesh_info

      if mesh_info:
        type_options['BlockName'] = mesh_info.blockNames()
        type_options['std::vector<BlockName>'] = mesh_info.blockNames()
        type_options['BoundaryName'] = mesh_info.sidesetNames()
        print 'Adding boundary names:', mesh_info.sidesetNames()
        type_options['std::vector<BoundaryName>'] = mesh_info.sidesetNames()
        type_options['BoundaryName'].update(mesh_info.nodesetNames())
        type_options['std::vector<BoundaryName>'].update(mesh_info.sidesetNames())
        type_options['SubdomainName'].update(mesh_info.blockNames())
        type_options['std::vector<SubdomainName>'].update(mesh_info.blockNames())
      
    return type_options
    
  def _doubleClickedItem(self, item, column):
    # Make sure the syntax is up to date
    self.input_file_widget.recache()
    
    this_path = self.generatePathFromItem(item)

    if not self.action_syntax.isPath(this_path) or self._itemHasEditableParameters(item):
      already_had_data = False
      try:
        item.table_data # If this fails we will jump to "except"...
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

      new_gui = OptionsGUI(yaml_entry, self.action_syntax, item.text(column), item.table_data, item.param_comments, item.comment, False, self._typeOptions())

      if item.table_data:
        new_gui.incoming_data = item.table_data

      if new_gui.exec_():
        item.table_data = new_gui.result()
        item.param_comments = new_gui.param_table.param_comments
        item.comment = new_gui.param_table.comment
        if not self.action_syntax.isPath(this_path):  # Don't change the name of hard paths
          item.setText(0,item.table_data['Name'])
        item.setCheckState(0, QtCore.Qt.Checked)
        if item.text(0) == 'Mesh':
          self.mesh_item_changed.emit(item)
        self._updateOtherGUIElements()

  def _itemChanged(self, item, column):
    self._updateOtherGUIElements()

  def _deleteCurrentItem(self):
    item = self.currentItem()
    parent = item.parent()
    if parent:
      parent.removeChild(item)
    else: #Must be a top level item
      self.removeItemWidget(item, 0)
    self.addHardPathsToTree() # We do this here because they might have removed a hard path... but there is no way to get them back
    self._updateOtherGUIElements()

  def _editComment(self):
    item = self.currentItem()
    ce = CommentEditor(item)
    if ce.exec_():
      self._itemChanged(item, 0)

  def _addItem(self):
    item = self.currentItem()
    this_path = self.generatePathFromItem(item)
    this_path = '/' + self.action_syntax.getPath(this_path) # Get the real action path associated with this item
    yaml_entry = self.input_file_widget.yaml_data.findYamlEntry(this_path)

    self.new_gui = OptionsGUI(yaml_entry, self.action_syntax, item.text(0), None, None, None, False, self._typeOptions())
    if self.new_gui.exec_():
      table_data = self.new_gui.result()
      param_comments = self.new_gui.param_table.param_comments
      comment = self.new_gui.param_table.comment
      new_child = QtGui.QTreeWidgetItem(item)
      new_child.setText(0,table_data['Name'])
      new_child.table_data = table_data
      new_child.param_comments = param_comments
      new_child.comment = comment
      new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      new_child.setCheckState(0, QtCore.Qt.Checked)
      item.addChild(new_child)
      item.setCheckState(0, QtCore.Qt.Checked)
      item.setExpanded(True)
      self.setCurrentItem(new_child)
      
      if item.text(0) == 'Mesh':
        self.mesh_item_changed.emit(item)
        
      self._updateOtherGUIElements()
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

    comment_action = QtGui.QAction("Edit Comment...", self)
    comment_action.triggered.connect(self._editComment)
    menu.addAction(comment_action)
      
    menu.popup(global_pos)

  def _updateOtherGUIElements(self):
    self.tree_changed.emit()
    self.input_file_widget.input_file_textbox.updateTextBox()    
      
  def _currentItemChanged(self, current, previous):
    if not current:
      return
    
    if 'boundary' in current.table_data:
      self.input_file_widget.mesh_render_widget.highlightBoundary(current.table_data['boundary'])
    elif 'master' in current.table_data:
      if 'slave' in current.table_data:
        self.input_file_widget.mesh_render_widget.highlightBoundary(current.table_data['master']+' '+current.table_data['slave'])
    elif 'block' in current.table_data:
      self.input_file_widget.mesh_render_widget.highlightBlock(current.table_data['block'])
    elif previous and ('boundary' in previous.table_data or 'block' in previous.table_data or ('master' in previous.table_data and 'slave' in previous.table_data)):
      self.input_file_widget.mesh_render_widget.clearHighlight()
#    except:
#      pass
      
