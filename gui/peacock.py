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
  
  def init_treewidet(self, layout):
    i = 0
    self.tree_widget = QtGui.QTreeWidget()
    #Go over each top level section and add it to the tree
    for itm in self.main_data:
      new_child = QtGui.QTreeWidgetItem(self.tree_widget)
      new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
      new_child.setCheckState(0, QtCore.Qt.Unchecked)
      name = itm['name'].split('/')[1]
#      if 'subblocks' in itm and itm['subblocks'] != None and len(itm['subblocks']) > 0:
#        name += '+'
      new_child.setText(0, name)
      
    self.tree_widget.header().close()
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemDoubleClicked(QTreeWidgetItem *, int)"), self.input_selection)
    QtCore.QObject.connect(self.tree_widget, QtCore.SIGNAL("itemChanged(QTreeWidgetItem*, int)"), self.item_changed)
    self.tree_widget.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
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
    table_data = node.params
    table_data['Name'] = node.name
    parent_item.setCheckState(0, QtCore.Qt.Checked)
    new_child = QtGui.QTreeWidgetItem(parent_item)
    new_child.setText(0,table_data['Name'])
    new_child.table_data = table_data
    new_child.setFlags(QtCore.Qt.ItemIsSelectable | QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsUserCheckable)
    new_child.setCheckState(0, QtCore.Qt.Checked)
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

        num_params = 0
        for param in section_node.params:
          if param != 'active':
            num_params += 1
        
        if num_params > 0:
          new_name = 'ParentParams'
          new_node = GPNode(new_name, section_node)
          new_node.params = section_node.params
          new_node.params['parent_params'] = True
          if section_name == 'Mesh' and not 'type' in new_node.params:
            new_node.params['type'] = 'MooseMesh'
          
          self.addDataRecursively(self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0], new_node)
          
        for child, child_node in section_node.children.items():
          self.addDataRecursively(self.tree_widget.findItems(section_name, QtCore.Qt.MatchExactly)[0], child_node)
      self.input_display.setText(self.buildInputString())
    
  def click_cancel(self):
    sys.exit(0)

  def inputStringRecurse(self, item, level):
    indent_string = ''
    for i in xrange(0,level):
      indent_string += '  '

    if item.checkState(0) != QtCore.Qt.Checked:
      return
    else:
      section = str(item.text(0)).rstrip('+')
      subchild_count = item.childCount()

      if level == 0:
        self.the_string += '[' + section + ']\n'
      else:
        self.the_string += indent_string + '[./' + section + ']\n'

#       active = []
#       has_inactive_children = True # Whether or not it has inactive children
      
#       # Print the active line if necessary
#       for j in range(subchild_count):
#         subitem = item.child(j)
#         if subitem.checkState(0) == QtCore.Qt.Checked:
        

      # Print out the params for this item
      try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
        for param,value in item.table_data.items():
          if param != 'Name' and param != 'parent_params':
            self.the_string += indent_string + '  ' + param + ' = ' + value + '\n'
      except:
        pass

      # Grab all the subblocks with "parent_params" and print them out
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.checkState(0) != QtCore.Qt.Checked:
          continue

        table_data = subitem.table_data

        if 'parent_params' in table_data:
          for param,value in table_data.items():
            if param != 'Name' and param != 'parent_params':
              self.the_string += indent_string + '  ' + param + ' = ' + value + '\n'
          break

      # Now recurse over the children that _don't_ have parent_params
      for j in range(subchild_count):
        subitem = item.child(j)
        if subitem.checkState(0) != QtCore.Qt.Checked:
          continue
        table_data = subitem.table_data

        if 'parent_params' in table_data:
          continue

        self.inputStringRecurse(subitem, level+1)

      if level == 0:
        self.the_string += '[]\n'
      else:
        self.the_string += indent_string + '[../]\n'

    
  def buildInputString(self):
    self.the_string = ''
    root = self.tree_widget.invisibleRootItem()
    child_count = root.childCount()
    for i in range(child_count):
      item = root.child(i)
      self.inputStringRecurse(item, 0)
    return self.the_string

  def click_save(self):
    file_name = QtGui.QFileDialog.getSaveFileName(self, "Save Input File", "~/", "Input Files (*.i)")

    if file_name != '':
      file = open(file_name,'w')
      output_string = self.buildInputString()
      file.write(output_string)

  def input_selection(self, item, column):
    try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
      print 'Here 3'
      item.table_data
      print 'Here 4'
      for sgl_item in self.main_data:
        if sgl_item['name'].find('/'+str(item.parent().text(column)).rstrip('+')) != -1:
          print 'Here 5'
          new_gui = OptionsGUI(sgl_item, str(item.text(column)).rstrip('+'), item.table_data)
          new_gui.incoming_data = item.table_data
          if new_gui.exec_():
            table_data = new_gui.result()
            item.table_data = table_data
            item.setText(0,table_data['Name'])
            self.input_display.setText(self.buildInputString())
    except:
      for sgl_item in self.main_data:
        if sgl_item['name'].find('/'+str(item.text(column)).rstrip('+')) != -1:
          self.new_gui = OptionsGUI(sgl_item, str(item.text(column)).rstrip('+'), None)
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
          break
        
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
