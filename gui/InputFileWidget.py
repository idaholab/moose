#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *
from ParamTable import *

from readInputFile import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileWidget(QtGui.QWidget):
  def __init__(self, app_path, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.main_data = GenSyntax(app_path).GetSyntax()
    self.action_syntax = ActionSyntax(app_path)

    # Start with an input file template if this application has one
    input_file_template_name = os.path.dirname(app_path) + '/input_template'
    if os.path.isfile(input_file_template_name):
      self.input_file_template_root_node = readInputFile(input_file_template_name)
      
    self.input_file_root_node = None

    self.constructed_data = {}
    self.initUI()
    
  def newEditParamWidget(self):
    try:
      self.edit_param_widget.hide()
      self.edit_param_layout_spot.removeWidget(self.edit_param_widget)
    except:
      pass
    
    self.edit_param_widget = QtGui.QWidget(self)
    self.edit_param_widget.hide()
    self.edit_param_layout = QtGui.QVBoxLayout(self.edit_param_widget)
    self.edit_param_layout_spot.addWidget(self.edit_param_widget)
    
  def initUI(self):
    # Just a holder so the edit param_widget can go in where we want
    self.edit_param_layout_spot = QtGui.QVBoxLayout()
    
    self.layoutV = QtGui.QVBoxLayout()
    self.layoutH = QtGui.QHBoxLayout()
    self.layout_with_textbox = QtGui.QHBoxLayout()
    self.init_treewidet(self.layoutV)
    self.init_buttons(self.layoutH)
    self.layoutV.addLayout(self.layoutH)
    self.layout_with_textbox.addLayout(self.layoutV)
    self.layout_with_textbox.addLayout(self.edit_param_layout_spot)
    self.init_textbox()    
    self.setLayout(self.layout_with_textbox)

  def init_textbox(self):
    self.input_display = QtGui.QTextEdit()
    self.input_display.setMinimumWidth(300)
    self.input_display.setReadOnly(True)
    self.textbox_layout = QtGui.QVBoxLayout()
    self.textbox_layout.addWidget(self.input_display)
    self.textbox_layout.setSizeConstraint(QtGui.QLayout.SetMinimumSize)
    self.layout_with_textbox.addLayout(self.textbox_layout)

  def recursivelyAddTreeItems(self, split_path, parent):
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

    if len(split_path) > 1:
      if not is_star:
        self.recursivelyAddTreeItems(split_path[1:], this_item)
      else: # If it is a star and there are children - then add it to all of the children
        for i in range(num_children):
          child = None
          try: # This will fail when we're dealing with the QTreeWidget itself
            child = parent.child(i)
          except:
            child = parent.topLevelItem(i)
          self.recursivelyAddTreeItems(split_path[1:], child)

  def addHardPathsToTree(self):
    # Add every hard path
    for path in self.action_syntax.hard_paths:
      self.recursivelyAddTreeItems(path.split('/'), self.tree_widget)
    
  def init_treewidet(self, layout):
    i = 0
    self.tree_widget = QtGui.QTreeWidget()
    self.tree_widget.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
    self.connect(self.tree_widget,QtCore.SIGNAL('customContextMenuRequested(QPoint)'), self.newContext)
    self.addHardPathsToTree()
      
    self.tree_widget.header().close()
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemDoubleClicked(QTreeWidgetItem *, int)"), self.input_selection)
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"), self.item_changed)
    layout.addWidget(self.tree_widget)

  def init_buttons(self, layout):
    self.buttonOpen = QtGui.QPushButton("Open")
    self.buttonSave = QtGui.QPushButton("Save")
    self.buttonClear = QtGui.QPushButton("Clear")
    QtCore.QObject.connect(self.buttonOpen, QtCore.SIGNAL("clicked()"), self.click_open)
    QtCore.QObject.connect(self.buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)
    QtCore.QObject.connect(self.buttonClear, QtCore.SIGNAL("clicked()"), self.click_clear)
    layout.addWidget(self.buttonOpen)
    layout.addWidget(self.buttonSave)
    layout.addWidget(self.buttonClear)

  def addDataRecursively(self, parent_item, node):
    is_active = 'active' not in node.parent.params or node.parent.params['active'].find(node.name) != -1
    table_data = node.params
    table_data['Name'] = node.name
    new_child = QtGui.QTreeWidgetItem(parent_item)
    new_child.setText(0,table_data['Name'])

    # If this is a hard path then we need to add ParentParams for it
    if self.action_syntax.isPath(self.generatePathFromItem(new_child)):
      num_params = 0
      for param in node.params:
        if param != 'active':
          num_params += 1

      if num_params > 0:
        new_name = 'ParentParams'
        new_node = GPNode(new_name, node)
        new_node.params = node.params
        new_node.params['parent_params'] = True
        parent_item.setCheckState(0, QtCore.Qt.Checked)
        self.addDataRecursively(new_child, new_node)
    else: # Otherwise this is just a normal node
      new_child.table_data = table_data
      new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)

    if is_active:
      new_child.setCheckState(0, QtCore.Qt.Checked)
    else:
      new_child.setCheckState(0, QtCore.Qt.Unchecked)
    parent_item.addChild(new_child)

    for child, child_node in node.children.items():
      self.addDataRecursively(new_child, child_node)
    
  def click_open(self):
    file_name = QtGui.QFileDialog.getOpenFileName(self, "Open Input File", "~/", "Input Files (*.i)")

    if file_name != '':
      os.chdir(os.path.dirname(str(file_name)))
      self.input_file_root_node = readInputFile(file_name)
      main_sections = self.input_file_root_node.children
      for section_name, section_node in main_sections.items():
        # Find out if this section has it's own parameters.  If so we need to add a ParentParams node

        section_item = self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0]

        num_params = 0
        for param in section_node.params:
          if param != 'active':
            num_params += 1

        if num_params > 1 or len(section_node.children):
          section_item.setCheckState(0, QtCore.Qt.Checked)
        
        if num_params > 0:
          new_name = 'ParentParams'
          new_node = GPNode(new_name, section_node)
          new_node.params = section_node.params
          new_node.params['parent_params'] = True
          if section_name == 'Mesh' and not 'type' in new_node.params:
            new_node.params['type'] = 'MooseMesh'

          section_item.setCheckState(0, QtCore.Qt.Checked)
          self.addDataRecursively(section_item, new_node)
          
        for child, child_node in section_node.children.items():
          self.addDataRecursively(self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0], child_node)
      self.input_display.setText(self.buildInputString())
      self.addHardPathsToTree() # We do this here because * paths might add more paths underneath some of the paths
    
  def click_clear(self):
    msgBox = QMessageBox();
    msgBox.setText("Clear Tree?");
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No);
    msgBox.setDefaultButton(QMessageBox.No);
    ret = msgBox.exec_();
    if ret == QMessageBox.Yes:
      self.tree_widget.clear()
      self.addHardPathsToTree()

  def recursiveGetGPNode(self, current_node, pieces):
#    print 'in rggpn',current_node.name, pieces
    if len(pieces) == 1 and pieces[0] == current_node.name:
      return current_node

    if pieces[1] in current_node.children:
      return self.recursiveGetGPNode(current_node.children[pieces[1]], pieces[1:])

    # See if there is a * here (this would be from an input template)
    if len(pieces) == 2 and '*' in current_node.children:
      return current_node.children['*']

    return None

  """ Returns the GetPot Node associated with the item... or None if there wasn't one """
  def getGPNode(self, root_gp_node, item):
    if not self.input_file_root_node and not self.input_file_template_root_node:
      return None
    else:
      path = self.generatePathFromItem(item)
      pieces = path.split('/')
      return self.recursiveGetGPNode(root_gp_node, pieces)

  def inputStringRecurse(self, item, level):    
    indent_string = ''
    for i in xrange(0,level):
      indent_string += '  '
      
    this_path = self.generatePathFromItem(item)

    # Don't print hard paths that aren't checked
    if self.action_syntax.isPath(this_path) and item.checkState(0) != QtCore.Qt.Checked:
      return
    
    else:
      section = str(item.text(0)).rstrip('+')
      subchild_count = item.childCount()
      gp_node = None
      template_gp_node = None

      if self.input_file_root_node:
        gp_node = self.getGPNode(self.input_file_root_node, item)

      if self.input_file_template_root_node:
        template_gp_node = self.getGPNode(self.input_file_template_root_node, item)
#        template_gp_node.Print()

      if level == 0:
        self.the_string += '[' + section + ']\n'
      else:
        self.the_string += indent_string + '[./' + section + ']\n'

      active = []
      has_inactive_children = False # Whether or not it has inactive children
      
      # Print the active line if necessary
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.checkState(0) == QtCore.Qt.Checked:
          active.append(str(subitem.text(0)))
        else:
          if not self.action_syntax.isPath(self.generatePathFromItem(subitem)) and str(subitem.text(0)) != 'ParentParams':
            has_inactive_children = True

      if has_inactive_children:
          self.the_string += indent_string + '  ' + "active = '" + ' '.join(active) + "'\n"

      # Print out the params for this item
      try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
        printed_params = []

        # Print out the ones that we know from the read in input file in the right order
        if gp_node:
          for param in gp_node.params_list:
            if param in item.table_data and param != 'Name' and param != 'parent_params':
              self.the_string += indent_string + '  ' + param + ' = ' + item.table_data[param] + '\n'
              printed_params.append(param)

        if template_gp_node:
          for param in template_gp_node.params_list:
            if param in item.table_data and param != 'Name' and param != 'parent_params' and param not in printed_params:
              self.the_string += indent_string + '  ' + param + ' = ' + item.table_data[param] + '\n'
              printed_params.append(param)
        
        for param,value in item.table_data.items():
          if param not in printed_params and param != 'Name' and param != 'parent_params':
            self.the_string += indent_string + '  ' + param + ' = ' + value + '\n'
      except:
        pass

      # Grab all the subblocks with "parent_params" and print them out
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.checkState(0) != QtCore.Qt.Checked:
          continue
        try:
          table_data = subitem.table_data

          printed_params = []

          # Print out the ones that we know from the read in input file in the right order
          if gp_node:
            for param in gp_node.params_list:
              if param in table_data and param != 'Name' and param != 'parent_params':
                self.the_string += indent_string + '  ' + param + ' = ' + table_data[param] + '\n'
                printed_params.append(param)

          if template_gp_node:
            for param in template_gp_node.params_list:
              if param in table_data and param != 'Name' and param != 'parent_params' and param not in printed_params:
                self.the_string += indent_string + '  ' + param + ' = ' + table_data[param] + '\n'
                printed_params.append(param)


          if 'parent_params' in table_data:
            for param,value in table_data.items():
              if param not in printed_params and param != 'Name' and param != 'parent_params':
                self.the_string += indent_string + '  ' + param + ' = ' + value + '\n'
            break
        except:
          pass

      # Now recurse over the children that _don't_ have parent_params
      printed_children = []
      if gp_node:  # Print the children we knew about from the input file in the right order
        for child in gp_node.children_list:
          for j in range(subchild_count):
            subitem = item.child(j)
            if subitem.text(0) != child:
              continue

            try:
              table_data = subitem.table_data

              if 'parent_params' in table_data:
                continue
            except:
              pass

            self.inputStringRecurse(subitem, level+1)
            printed_children.append(child)

      if template_gp_node:  # Print the children we knew about from the input file in the right order
        for child in template_gp_node.children_list:
          for j in range(subchild_count):
            subitem = item.child(j)
            if subitem.text(0) != child:
              continue

            if subitem.text(0) in printed_children:
              continue
            
            try:
              table_data = subitem.table_data

              if 'parent_params' in table_data:
                continue
            except:
              pass

            self.inputStringRecurse(subitem, level+1)
            printed_children.append(child)

      
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.text(0) in printed_children:
          continue
        
        try:
          table_data = subitem.table_data

          if 'parent_params' in table_data:
            continue
        except:
          pass

        self.inputStringRecurse(subitem, level+1)

    if level == 0:
      self.the_string += '[]\n\n'
    else:
      self.the_string += indent_string + '[../]\n'

    
  def buildInputString(self):
    self.the_string = ''
    root = self.tree_widget.invisibleRootItem()
    child_count = root.childCount()
    printed_sections = []
    
    if self.input_file_root_node: # Print any sections we knew about from the input file first (and in the right order)
      for section_name in self.input_file_root_node.children_list:
        search = self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)

        if search and len(search):
          item = search[0]
          self.inputStringRecurse(item, 0)
          printed_sections.append(section_name)  
        
    if self.input_file_template_root_node: # Print any sections we knew about from the template input file first (and in the right order)
      for section_name in self.input_file_template_root_node.children_list:
        if section_name in printed_sections:
          continue
        search = self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)

        if search and len(search):
          item = search[0]
          self.inputStringRecurse(item, 0)
          printed_sections.append(section_name)  

    for i in range(child_count): # Print out all the other sections
      item = root.child(i)
      if item.text(0) not in printed_sections:
        self.inputStringRecurse(item, 0)
        
    return self.the_string

  def click_save(self):
    file_name = QtGui.QFileDialog.getSaveFileName(self, "Save Input File", "~/", "Input Files (*.i)")

    if file_name != '':
      file = open(file_name,'w')
      output_string = self.buildInputString()
      file.write(output_string)
      os.chdir(os.path.dirname(str(file_name)))

  def generatePathFromItem(self, item):
    from_parent = ''
    if item.parent():
      from_parent = self.generatePathFromItem(item.parent())
      
    return from_parent + '/' + str(item.text(0))

  def recursiveYamlDataSearch(self, path, current_yaml):
    if current_yaml['name'] == path:
      return current_yaml
    else:
      if current_yaml['subblocks']:
        for child in current_yaml['subblocks']:
          yaml_data = self.recursiveYamlDataSearch(path, child)
          
          if yaml_data:  # Found it in a child!
            return yaml_data
      else: # No children.. stop recursion
        return None

  def findYamlEntry(self, path):
    for yaml_it in self.main_data:
      yaml_data = self.recursiveYamlDataSearch(path, yaml_it)

      if yaml_data:
        return yaml_data
      
    # This means it wasn't found  
    return None

  def deleteCurrentItem(self):
    item = self.tree_widget.currentItem()
    parent = item.parent()
    parent.removeChild(item)
    self.input_display.setText(self.buildInputString())
    
  def newContext(self, pos):
    global_pos = self.tree_widget.mapToGlobal(pos)
    item = self.tree_widget.itemAt(pos)
    this_path = self.generatePathFromItem(item)

    # Don't allow deletion of hard paths
    if not self.action_syntax.isPath(this_path):
      menu = QtGui.QMenu(self)
      delete_action = QtGui.QAction("Delete", self)
      delete_action.triggered.connect(self.deleteCurrentItem)
      menu.addAction(delete_action)
      menu.popup(global_pos)
  
  def input_selection(self, item, column):
    this_path = self.generatePathFromItem(item)
    
    try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
      item.table_data # If this fails we will jump to "except"...
      parent_path = self.generatePathFromItem(item.parent())
      parent_path = '/' + self.action_syntax.getPath(parent_path) # Get the real action path associated with this item
      yaml_entry = self.findYamlEntry(parent_path)
      # This stuff will edit the parameters _in_ the window!
#      self.newEditParamWidget()
#      self.param_table = ParamTable(yaml_entry, self.action_syntax, str(item.text(column)).rstrip('+'), item.table_data, self.edit_param_layout, self)
#      self.edit_param_widget.show()
      
      new_gui = OptionsGUI(yaml_entry, self.action_syntax, str(item.text(column)).rstrip('+'), item.table_data)
      new_gui.incoming_data = item.table_data
      if new_gui.exec_():
        item.table_data = new_gui.result()
        item.setText(0,item.table_data['Name'])
        self.input_display.setText(self.buildInputString())
    except AttributeError:
      this_path = '/' + self.action_syntax.getPath(this_path) # Get the real action path associated with this item
      yaml_entry = self.findYamlEntry(this_path)
      self.new_gui = OptionsGUI(yaml_entry, self.action_syntax, str(item.text(column)).rstrip('+'), None)
      if self.new_gui.exec_():
        table_data = self.new_gui.result()
        new_child = QtGui.QTreeWidgetItem(item)
        new_child.setText(0,table_data['Name'])
        new_child.table_data = table_data
        new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
        new_child.setCheckState(0, QtCore.Qt.Checked)
        item.addChild(new_child)
        item.setCheckState(0, QtCore.Qt.Checked)
        self.input_display.setText(self.buildInputString())
        self.addHardPathsToTree() # We do this here because * paths might add more paths underneath the item we just added
        
  def item_changed(self, item, column):
    self.input_display.setText(self.buildInputString())

  def accept_params(self):
    self.edit_param_widget.hide()
    
  def reject_params(self):
    self.edit_param_widget.hide()
