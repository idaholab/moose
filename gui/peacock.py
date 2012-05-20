#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui

from InputFileWidget import *
from ExecuteWidget import *


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
  def __init__(self, app_path, qt_app, win_parent = None):
    QtGui.QMainWindow.__init__(self, win_parent)
    self.setWindowTitle('Peacock - MOOSE front end')
    self.app_path = os.path.abspath(app_path)
    self.qt_app = qt_app

    self.initUI()
    
  def initUI(self):
    self.main_ui = QtGui.QWidget(self)
    self.main_ui.setObjectName(_fromUtf8("Dialog"))
    self.main_layout = QtGui.QVBoxLayout()
    self.setCentralWidget(self.main_ui)

    self.input_file_widget = InputFileWidget(self.app_path)
    self.execute_widget = ExecuteWidget(self.app_path, self.input_file_widget, self.qt_app)

    self.tab_widget = QtGui.QTabWidget()

    self.tab_widget.addTab(self.input_file_widget, "Input File")
    self.tab_widget.addTab(self.execute_widget, "Execute")
    
    self.main_layout.addWidget(self.tab_widget)
    
    self.main_ui.setLayout(self.main_layout)
    self.resize(700,800)
    
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
    main_window = UiBox(application,app)
    main_window.show()
    app.exec_()
  else:
    print 'Path not found:', application
    sys.exit(1)
