#!/usr/bin/python
import os, sys, PyQt4, getopt
from PyQt4 import QtCore, QtGui


from GenSyntax import *
from ActionSyntax import *
from YamlData import *
from GetPotData import *
from InputFileTreeWidget import *
from InputFileTextbox import *

from ParseGetPot import readInputFile, GPNode

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s

class InputFileWidget(QtGui.QWidget):
  def __init__(self, app_path, options, peacock_ui, qt_app, application, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path = app_path
    self.options = options
    self.peacock_ui = peacock_ui
    self.qt_app = qt_app
    self.application = application
    self.yaml_data = None

    self.recache()
    
    self.action_syntax = ActionSyntax(app_path)

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

    self.modifyUI()

  ''' This will be called after the interface is completely setup to allow an application to modify this tab '''
  def modifyUI(self):
    pass
    
  def initUI(self):
    # Just a holder so the edit param_widget can go in where we want
    self.edit_param_layout_spot = QtGui.QVBoxLayout()

    self.tree_widget_layout_widget = QtGui.QWidget()
    self.tree_widget_layout = QtGui.QVBoxLayout()
    self.tree_widget_layout_widget.setLayout(self.tree_widget_layout)
    self.layoutH = QtGui.QHBoxLayout()
    self.layout_with_textbox = QtGui.QSplitter()
#    self.layout_with_textbox.setChildrenCollapsible(False)

    self.input_file_textbox = InputFileTextbox(self)
#    self.input_file_textbox.hide()
    self.tree_widget = InputFileTreeWidget(self)
    
    self.tree_widget_layout.addWidget(self.tree_widget)
    self.init_buttons(self.layoutH)
    self.tree_widget_layout.addLayout(self.layoutH)
    self.layout_with_textbox.addWidget(self.tree_widget_layout_widget)
#    self.layout_with_textbox.addLayout(self.edit_param_layout_spot)

    self.mesh_render_widget = self.application.meshRenderWidget(self)
    if not self.application.showMeshRenderWidgetByDefault():
      self.mesh_render_widget.hide()
    self.layout_with_textbox.addWidget(self.mesh_render_widget)

    self.input_file_textbox_layout_widget = QtGui.QWidget()
    self.input_file_textbox_layout_widget.setLayout(self.input_file_textbox.getLayout())
    self.layout_with_textbox.addWidget(self.input_file_textbox_layout_widget)


    self.layout_with_textbox.setStretchFactor(0,0.1)
    self.layout_with_textbox.setStretchFactor(1,0.9)
    self.layout_with_textbox.setStretchFactor(1,0.2)

    self.layout_with_textbox.setSizes([30,600,0])

    self.main_layout = QtGui.QHBoxLayout()
    self.main_layout.addWidget(self.layout_with_textbox)
    self.setLayout(self.main_layout)

    self.menubar = self.peacock_ui.menuBar()

    # build menu
    self.file_menu = self.menubar.addMenu('&File')
    open_file_action = QtGui.QAction("Open...", self)
    open_file_action.setShortcut('Ctrl+O')
    open_file_action.triggered.connect(self.click_open)
    self.file_menu.addAction(open_file_action)
    save_file_action = QtGui.QAction("Save...", self)
    save_file_action.setShortcut('Ctrl+S')
    save_file_action.triggered.connect(self.click_save)
    self.file_menu.addAction(save_file_action)

    self.edit_menu = self.menubar.addMenu('&Edit')
    main_comment_action = QtGui.QAction("Main Comment", self)
    main_comment_action.triggered.connect(self._edit_main_comment)
    self.edit_menu.addAction(main_comment_action)    

    self.view_menu = self.menubar.addMenu('&View')
    input_file_action = QtGui.QAction("Input File", self)
    input_file_action.triggered.connect(self._view_input_file)
    self.view_menu.addAction(input_file_action)    
    mesh_view_action = QtGui.QAction("Mesh View", self)
    mesh_view_action.triggered.connect(self._view_mesh_view)
    self.view_menu.addAction(mesh_view_action)    

  ''' Return the name to use for this tab '''
  def name(self):
    return 'Input File'

  def init_buttons(self, layout):
    self.buttonOpen = QtGui.QPushButton("Open")
    self.buttonOpen.setToolTip("Open existing input file")
    self.buttonSave = QtGui.QPushButton("Save")
    self.buttonSave.setToolTip("Save current tree items to an input file")
    self.buttonClear = QtGui.QPushButton("Clear")
    self.buttonClear.setToolTip("Clear the current tree items")
    QtCore.QObject.connect(self.buttonOpen, QtCore.SIGNAL("clicked()"), self.click_open)
    QtCore.QObject.connect(self.buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)
    QtCore.QObject.connect(self.buttonClear, QtCore.SIGNAL("clicked()"), self.click_clear)
    layout.addWidget(self.buttonOpen)
    layout.addWidget(self.buttonSave)
    self.application.addRelapSave(layout)
    layout.addWidget(self.buttonClear)
    
  def getOutputFileNames(self):
    return self.tree_widget.getOutputFileNames()
    
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

      main_comment = '\n'.join(self.input_file_root_node.comments)
      
      self.tree_widget.comment = main_comment
      
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

  def _edit_main_comment(self):
    ce = CommentEditor(self.tree_widget)
    if ce.exec_():
      self.tree_widget._itemChanged(self.tree_widget, 0)

  def _view_input_file(self):
    if self.input_file_textbox.isVisible():
      self.input_file_textbox.hide()
      sizes = self.layout_with_textbox.sizes()
      sizes[2] = 0
      self.layout_with_textbox.setSizes(sizes)
    else:
      self.input_file_textbox.show()
      sizes = self.layout_with_textbox.sizes()
      sizes[2] = 50
      self.layout_with_textbox.setSizes(sizes)
    
  def _view_mesh_view(self):
    if self.mesh_render_widget.isVisible():
      self.mesh_render_widget.hide()
      sizes = self.layout_with_textbox.sizes()
      sizes[1] = 0
      self.layout_with_textbox.setSizes(sizes)
    else:
      self.mesh_render_widget.show()
      sizes = self.layout_with_textbox.sizes()
      sizes[1] = 600
      self.layout_with_textbox.setSizes(sizes)
    
  def _selected_recache(self):
    self.recache(True)

  def recache(self, force_recache = False):

    if not self.yaml_data:
      self.yaml_data = YamlData(self.qt_app, self.app_path, force_recache or self.options.recache)
    else:
      self.yaml_data.recache(False)
