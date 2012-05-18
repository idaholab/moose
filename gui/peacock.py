#!/usr/bin/python
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/Users/milljm/projects/moose_front_end/Peacock.ui'
#
# Created: Wed Apr 18 13:54:16 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *

from readInputFile import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

_USAGE = """

peacock.py application_name

Where application_name is the name of your applicaiton.
eg:

     ./peacock.py ../bison
or
     ./peacock.py /home/userid/projects/trunk/bison
"""

class UiBox(QtGui.QMainWindow):
  def __init__(self, app_path, win_parent = None):
    QtGui.QMainWindow.__init__(self, win_parent)
    self.setWindowTitle('Peacock - MOOSE front end')
    self.main_data = GenSyntax(app_path).GetSyntax()
    self.action_syntax = ActionSyntax(app_path)
    self.input_file_root_node = None

    self.constructed_data = {}
    self.initUI()
    self.input_file_root_node = None  #This will only not be None if you open an input file

  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
    self.setCentralWidget(self.main_ui)
    self.layoutV = QtGui.QVBoxLayout()
    self.layoutH = QtGui.QHBoxLayout()
    self.layout_with_textbox = QtGui.QHBoxLayout()
    self.init_treewidet(self.layoutV)
    self.init_buttons(self.layoutH)
    self.layoutV.addLayout(self.layoutH)
    self.layout_with_textbox.addLayout(self.layoutV)
    self.init_textbox()
    self.main_ui.setLayout(self.layout_with_textbox)
    self.resize(700,800)

  def init_textbox(self):
    self.input_display = QtGui.QTextEdit()
    self.input_display.setReadOnly(True)
    self.layout_with_textbox.addWidget(self.input_display)

  def recursivelyAddTreeItems(self, split_path, parent):
    this_piece = split_path[0]

    #FIXME: Deal with this!
    if this_piece == '*':
      return

    this_item = None
    search = self.tree_widget.findItems(this_piece, QtCore.Qt.MatchExactly)

    if len(search) > 0:
      # Already have this in the tree
      this_item = search[0]
    else:    
      # Add it
      this_item = QtGui.QTreeWidgetItem(parent)
      this_item.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      this_item.setCheckState(0, QtCore.Qt.Unchecked)
      this_item.setText(0, this_piece)

    if len(split_path) > 1:
      self.recursivelyAddTreeItems(split_path[1:], this_item)
  
  def init_treewidet(self, layout):
    i = 0
    self.tree_widget = QtGui.QTreeWidget()
    self.tree_widget.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
    self.connect(self.tree_widget,QtCore.SIGNAL('customContextMenuRequested(QPoint)'), self.newContext)
    # Add every hard path
    for path in self.action_syntax.hard_paths:
      self.recursivelyAddTreeItems(path.split('/'), self.tree_widget)
      
    self.tree_widget.header().close()
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemDoubleClicked(QTreeWidgetItem *, int)"), self.input_selection)
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"), self.item_changed)
    layout.addWidget(self.tree_widget)

  def init_buttons(self, layout):
    buttonOpen = QtGui.QPushButton("Open")
    buttonSave = QtGui.QPushButton("Save")
    buttonCancel = QtGui.QPushButton("Cancel")
    QtCore.QObject.connect(buttonOpen, QtCore.SIGNAL("clicked()"), self.click_open)
    QtCore.QObject.connect(buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)
    QtCore.QObject.connect(buttonCancel, QtCore.SIGNAL("clicked()"), self.click_cancel)
    layout.addWidget(buttonOpen)
    layout.addWidget(buttonSave)
    layout.addWidget(buttonCancel)

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

          self.addDataRecursively(section_item, new_node)
          
        for child, child_node in section_node.children.items():
          self.addDataRecursively(self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0], child_node)
      self.input_display.setText(self.buildInputString())
    
  def click_cancel(self):
    sys.exit(0)

  def recursiveGetGPNode(self, current_node, pieces):
    if len(pieces) == 1 and pieces[0] == current_node.name:
      return current_node

    if pieces[1] in current_node.children:
      return self.recursiveGetGPNode(current_node.children[pieces[1]], pieces[1:])

    return None

  """ Returns the GetPot Node associated with the item... or None if there wasn't one """
  def getGPNode(self, item):
    if not self.input_file_root_node:
      return None
    else:
      path = self.generatePathFromItem(item)
      pieces = path.split('/')
      return self.recursiveGetGPNode(self.input_file_root_node, pieces)

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

      if self.input_file_root_node:
        gp_node = self.getGPNode(item)

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
        item = self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0]
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

  def generatePathFromItem(self, item):
    from_parent = ''
    if item.parent():
      from_parent = self.generatePathFromItem(item.parent())
      
    return from_parent + '/' + str(item.text(0))

  def recursiveYamlDataSearch(self, path, current_yaml):
    if current_yaml['name'].find('Variables') != -1:
      print 'Current Yaml', current_yaml['name']
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
      print 'Here 3'
      item.table_data # If this fails we will jump to "except"...
      print "Old", this_path
      print 'Here 4'
      parent_path = self.generatePathFromItem(item.parent())
      parent_path = '/' + self.action_syntax.getPath(parent_path) # Get the real action path associated with this item
      print parent_path
      yaml_entry = self.findYamlEntry(parent_path)
      print 'Here 5'
      new_gui = OptionsGUI(yaml_entry, self.action_syntax, str(item.text(column)).rstrip('+'), item.table_data)
      new_gui.incoming_data = item.table_data
      if new_gui.exec_():
        item.table_data = new_gui.result()
        item.setText(0,item.table_data['Name'])
        self.input_display.setText(self.buildInputString())
    except AttributeError:
      print "New", this_path
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
        
  def item_changed(self, item, column):
    self.input_display.setText(self.buildInputString())
    
def printUsage(message):
  sys.stderr.write(_USAGE)
  if message:
    sys.exit('\nFATAL ERROR: ' + message)
  else:
    sys.exit(1)

def process_args():
  try:
    placeholder, opts = getopt.getopt(sys.argv[1:], '', ['help'])
  except getopt.GetoptError:
    printUsage('Invalid arguments.')
  if not opts:
    printUsage('No options specified')
  try:
    if (opts[0] == ''):
      printUsage('Invalid arguments.')
  except:
    printUsage('Invalid arguments.')
  return opts[0]


if __name__ == '__main__':
  application = process_args()
  if os.path.exists(application):
    app = QtGui.QApplication(sys.argv)
    main_window = UiBox(application)
    main_window.show()
    app.exec_()
  else:
    print 'Path not found:', application
    sys.exit(1)
