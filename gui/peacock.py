# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/Users/milljm/projects/moose_front_end/Peacock.ui'
#
# Created: Wed Apr 18 13:54:16 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

import sys, PyQt4, subprocess, yaml, time
from PyQt4 import QtCore, QtGui
try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class UiBox(QtGui.QMainWindow):
  def __init__(self, win_parent = None):
    QtGui.QMainWindow.__init__(self, win_parent)
    self.setWindowTitle('Peacock - MOOSE front end')
    self.elements = {}
    self.element_table = []
    self.initUI()

  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
    self.setCentralWidget(self.main_ui)
    self.layoutV = QtGui.QVBoxLayout()
    self.layoutH = QtGui.QHBoxLayout()
    self.init_treeview(self.layoutV)
    self.init_buttons(self.layoutH)
    self.layoutV.addLayout(self.layoutH)
    self.main_ui.setLayout(self.layoutV)

  def init_treeview(self, layout):
    tree_view = QtGui.QTreeView()
    layout.addWidget(tree_view)

  def init_buttons(self, layout):
    buttonSave = QtGui.QPushButton("Save")
    buttonCancel = QtGui.QPushButton("Cancel")
    QtCore.QObject.connect(buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)
    QtCore.QObject.connect(buttonCancel, QtCore.SIGNAL("clicked()"), self.click_cancel)
    QtCore.QObject.connect(buttonTest, QtCore.SIGNAL("clicked()"), self.click_test)
    layout.addWidget(buttonSave)
    layout.addWidget(buttonCancel)
    layout.addWidget(buttonTest)

  def GetSyntax(self, app_path, app_name):
    EXTENSIONS = [ 'opt', 'dbg', 'pro' ]
    fname = None
    timestamp = time.time() + 99 #initialize to a big number (in the future)
    for ext in EXTENSIONS:
      exe = app_path + '/' + app_name + '-' + ext
      if os.path.isfile(exe):
        if os.path.getmtime(exe) < timestamp:
          fname = exe
    if fname == None:
      print 'ERROR: You must build a ' + \
            app_name + ' executable in ' + app_path + ' first.'
      sys.exit(1)
    data = commands.getoutput( fname + " --yaml" )

  def click_cancel(self):
    for widget in self.elements:
      if widget == 'test' and self.elements[widget] != None:
        self.elements[widget].setParent(None)
        self.elements[widget] = None

  def click_save(self):
    self.pop_up.pop()
    print 'saved'

if __name__ == '__main__':
  app = QtGui.QApplication(sys.argv)
  main_window = UiBox()
  main_window.show()
  app.exec_()
