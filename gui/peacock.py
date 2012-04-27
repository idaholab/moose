# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file '/Users/milljm/projects/moose_front_end/Peacock.ui'
#
# Created: Wed Apr 18 13:54:16 2012
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!
import os, sys, PyQt4, commands, yaml, time, getopt, pickle
from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *
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

class OptionsGUI(QtGui.QMainWindow):
  def __init__(self, main_data, single_item, win_parent=None):
    QtGui.QMainWindow.__init__(self, win_parent)
    self.main_data = main_data
    self.single_item = single_item
    self.initUI()

  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
    self.setCentralWidget(self.main_ui)
    self.layoutV = QtGui.QVBoxLayout()
    self.init_menu(self.layoutV)
    self.main_ui.setLayout(self.layoutV)

  def init_menu(self, layout):
    drop_menu = QtGui.QComboBox()
    for item in self.main_data:
      drop_menu.addItem(item['name'].split('/').pop())
    drop_menu.activated[str].connect(self.item_clicked)
    layout.addWidget(drop_menu)

  def item_clicked(self, item):
    widget_list = []
#   edit_box = QtGui.QPlainTextEdit()
    for new_text in self.main_data:
      if new_text['name'].split('/').pop() == item:
        for sub_item in new_text['parameters']:
          widget_list.append(QtGui.QLabel())
          widget_list[len(widget_list) - 1].setText(sub_item['name'])
          self.layoutV.addWidget(widget_list[len(widget_list) - 1])
#         edit_box.appendPlainText(sub_item['name'])
#   self.widget_list.append(self.layoutV.addWidget(edit_box))
    self.main_ui.setLayout(self.layoutV)



class UiBox(QtGui.QMainWindow):
  def __init__(self, app_path, win_parent = None):
    QtGui.QMainWindow.__init__(self, win_parent)
    self.setWindowTitle('Peacock - MOOSE front end')
    self.app_path = app_path
    self.main_data = self.GetSyntax()
    self.initUI()

  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
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

  def GetSyntax(self):
    exet = os.path.split(self.app_path)
    if exet[len(exet) - 1] != '':
      exet = exet[len(exet) - 1].split('/').pop()
    else:
      exet = exet[0].split('/').pop()
    EXTENSIONS = [ 'opt', 'dbg', 'pro' ]
    fname = None
    timestamp = time.time() + 99 #initialize to a big number (in the future)
    for ext in EXTENSIONS:
      exe = self.app_path + '/' + exet + '-' + ext
      print exe
      if os.path.isfile(exe):
        if os.path.getmtime(exe) < timestamp:
          fname = exe
    if fname == None:
      print 'ERROR: You must build an ' + \
            'executable in ' + self.app_path + ' first.'
      sys.exit(1)
    data = commands.getoutput( fname + " --yaml" )
    data = data.split('**START YAML DATA**\n')[1]
    data = data.split('**END YAML DATA**')[0]
    if not os.path.exists('yaml_dump'):
      data = yaml.load(data)
      pickle.dump(data, open('yaml_dump', 'wb'))
    else:
      data = pickle.load(open('yaml_dump', 'rb'))
    data = self.massage_data(data)
    return data

  def massage_data(self, data):
    for block in data:
      name =  block['name']
      if name == 'Executioner' or name == 'InitialCondition':
        curr_type = str(block['type'])
        if curr_type == 'None':
          curr_type = 'ALL'
        block['name'] += ' (' + curr_type + ')'
    return data

  def click_cancel(self):
    sys.exit(0)

  def click_save(self):
    print 'saved'

  def input_selection(self, item, column):
    for sgl_item in self.main_data:
      if sgl_item['name'] == item.text(column) and sgl_item['subblocks'] != None:
        self.new_gui = OptionsGUI(sgl_item['subblocks'], item.text(column))
        self.new_gui.show()
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







#       try:
#         for sgl_child in itm['subblocks']:
#           if sgl_child['type'] != None:
#             temp_item = QtGui.QTreeWidgetItem()
#             iter_dict[i].addChild(temp_item)
#             temp_item.setText(0, sgl_child['type'])
#       except:
#         continue
