#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from OptionsGUI import OptionsGUI
from GenSyntax import *
from ActionSyntax import *
from YamlData import *
from GetPotData import *
from InputFileTreeWidget import *
from InputFileTextbox import *

from readInputFile import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileWidget(QtGui.QWidget):
  def __init__(self, app_path, options, peacock_ui, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path = app_path
    self.options = options

    self._recache(False)
    
    self.action_syntax = ActionSyntax(app_path)
    self.peacock_ui = peacock_ui

    # Start with an input file template if this application has one
    input_file_template_name = os.path.dirname(app_path) + '/input_template'
    self.input_file_template_root_node = None
    if os.path.isfile(input_file_template_name):
      self.input_file_template_root_node = readInputFile(input_file_template_name)
      self.input_file_template_getpot_data = GetPotData(self.input_file_template_root_node, self)
    else: # If they haven't specified their own template... let's use a default one:
      input_file_template_name = os.path.dirname(sys.argv[0]) + '/input_template'
      self.input_file_template_root_node = readInputFile(input_file_template_name)
      self.input_file_template_getpot_data = GetPotData(self.input_file_template_root_node, self)
      
    self.input_file_root_node = None

    self.constructed_data = {}
    self.initUI()
    if options.input_file:
      abs_input_file = os.path.abspath(options.input_file)
      if os.path.isfile(abs_input_file):
        self.openInputFile(abs_input_file)
      else:
        msgBox = QMessageBox()
        msgBox.setText("Warning: Input file, " + options.input_file + ", not found!")
        msgBox.setStandardButtons(QMessageBox.Ok)
        msgBox.setDefaultButton(QMessageBox.Ok)
        msgBox.exec_()    
    
  def initUI(self):
    # Just a holder so the edit param_widget can go in where we want
    self.edit_param_layout_spot = QtGui.QVBoxLayout()
    
    self.tree_widget_layout = QtGui.QVBoxLayout()
    self.layoutH = QtGui.QHBoxLayout()
    self.layout_with_textbox = QtGui.QHBoxLayout()

    self.input_file_textbox = InputFileTextbox(self)
    self.tree_widget = InputFileTreeWidget(self)
    
    self.tree_widget_layout.addWidget(self.tree_widget)
    self.init_buttons(self.layoutH)
    self.tree_widget_layout.addLayout(self.layoutH)
    self.layout_with_textbox.addLayout(self.tree_widget_layout)
    self.layout_with_textbox.addLayout(self.edit_param_layout_spot)
    self.layout_with_textbox.addLayout(self.input_file_textbox.getLayout())
    self.setLayout(self.layout_with_textbox)

    self.menubar = self.peacock_ui.menuBar()
    self.advanced_menu = self.menubar.addMenu('&Advanced')
    recache_action = QtGui.QAction("Recache Syntax", self)
    recache_action.triggered.connect(self._recache)
    self.advanced_menu.addAction(recache_action)

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
    
  def openInputFile(self, file_name):
    if file_name and file_name != '':
      progress = QtGui.QProgressDialog("Reading Input File...", "Abort", 0, 100, self)
      progress.setWindowModality(Qt.WindowModal)
      counter = 0

      counter+=1
      progress.setValue(counter)

      # Clear the tree
      self.tree_widget.clear()

      counter+=1
      progress.setValue(counter)

      self.tree_widget.addHardPathsToTree()

      counter+=1
      progress.setValue(counter)

      os.chdir(os.path.dirname(str(file_name)))

      counter+=1
      progress.setValue(counter)

      self.input_file_root_node = readInputFile(file_name)
      self.input_file_getpot_data = GetPotData(self.input_file_root_node, self)

      counter+=1
      progress.setValue(counter)
      
      main_sections = self.input_file_root_node.children

      self.tree_widget.loadData(counter, progress, main_sections)

  def click_open(self):
    file_name = QtGui.QFileDialog.getOpenFileName(self, "Open Input File", "~/", "Input Files (*.i)")
    if file_name:
      self.tree_widget.clear()
      self.tree_widget.addHardPathsToTree()
      self.openInputFile(file_name)
    
  def click_clear(self):
    msgBox = QMessageBox()
    msgBox.setText("Clear Tree?")
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No)
    msgBox.setDefaultButton(QMessageBox.No)
    ret = msgBox.exec_()
    if ret == QMessageBox.Yes:
      self.tree_widget.clear()
      self.tree_widget.addHardPathsToTree()


  def click_save(self):
    file_name = QtGui.QFileDialog.getSaveFileName(self, "Save Input File", "~/", "Input Files (*.i)")

    if file_name != '':
      file = open(file_name,'w')
      output_string = self.input_file_textbox.buildInputString()
      file.write(output_string)
      os.chdir(os.path.dirname(str(file_name)))    

  def _recache(self, force_recache = True):
    progress = QtGui.QProgressDialog("Caching Syntax...", "Abort", 0, 4, self)
    progress.setWindowModality(Qt.WindowModal)
    progress.setValue(2)

    self.yaml_data = YamlData(self.app_path, force_recache or self.options.recache)

    progress.setValue(3)
    progress.setValue(4)
