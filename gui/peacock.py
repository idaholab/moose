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
from GenSyntax import GenSyntax

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

  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
    self.main_ui.setGeometry(200, 500, 100, 100)
    self.setCentralWidget(self.main_ui)
    self.layoutV = QtGui.QVBoxLayout()
    self.layoutH = QtGui.QHBoxLayout()
    self.init_treewidet(self.layoutV)
    self.init_buttons(self.layoutH)
    self.layoutV.addLayout(self.layoutH)
    self.main_ui.setLayout(self.layoutV)

  def init_treewidet(self, layout):
    iter_dict = []
    i = 0
    tree_widget = QtGui.QTreeWidget()
    for itm in self.main_data:
      iter_dict.append(QtGui.QTreeWidgetItem(tree_widget))
      iter_dict[i].setText(0, itm['name'])
      i += 1
    QtCore.QObject.connect(tree_widget, QtCore.SIGNAL("itemDoubleClicked(QTreeWidgetItem *, int)"), self.input_selection)
    layout.addWidget(tree_widget)

  def init_buttons(self, layout):
    buttonSave = QtGui.QPushButton("Save")
    buttonCancel = QtGui.QPushButton("Cancel")
    QtCore.QObject.connect(buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)
    QtCore.QObject.connect(buttonCancel, QtCore.SIGNAL("clicked()"), self.click_cancel)
    layout.addWidget(buttonSave)
    layout.addWidget(buttonCancel)

  def click_cancel(self):
    sys.exit(0)

  def click_save(self):
    print 'saved'
    
  def input_selection(self, item, column):
    try: # Need to see if this item has data on it.  If it doesn't then we're creating a new item.
      item.table_data
      print item.table_data
      for sgl_item in self.main_data:
        if sgl_item['name'] == item.parent().text(column) and sgl_item['subblocks'] != None:
          new_gui = OptionsGUI(sgl_item['subblocks'], item.text(column), item.table_data)
          new_gui.incoming_data = item.table_data
          if new_gui.exec_():
            table_data = new_gui.result()
            item.table_data = table_data
    except:
      for sgl_item in self.main_data:
        if sgl_item['name'] == item.text(column) and sgl_item['subblocks'] != None:
          self.new_gui = OptionsGUI(sgl_item['subblocks'], item.text(column), None)
          if self.new_gui.exec_():
            table_data = self.new_gui.result()
            new_child = QtGui.QTreeWidgetItem(item)
            new_child.setText(0,table_data['Name'])
            new_child.table_data = table_data
            item.addChild(new_child)
          break

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
