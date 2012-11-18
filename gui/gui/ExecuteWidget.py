#!/usr/bin/python
import os, sys, PyQt4, getopt, subprocess
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

class ExecuteWidget(QtGui.QWidget):
  '''
  class containing the general Peacock execute tab
  '''
  #signal available
  oneRunInfoSetCopied     = QtCore.pyqtSignal()
  run_started             = QtCore.pyqtSignal()  
  run_stopped             = QtCore.pyqtSignal()
  timestep_begin          = QtCore.pyqtSignal()
  timestep_end            = QtCore.pyqtSignal()
  
  def __init__(self, app_path, input_file_widget, qt_app, win_parent=None):
    #set up the run info (multi simulation and one run setting) to default
    self.oneRunInfoSet('deafaultInMain')
    #setting up simulation flags
    self.oneRunInfoSetSaved   = False
    self.multiRunInfoSetSaved = False
    
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path          = app_path
    self.input_file_widget = input_file_widget
    self.qt_app            = qt_app
    self.isItRunning       = False 

    #layouts.....

    #page top layout
    self.topLayout()

    #the central page widgets
    self.pb_layout = QtGui.QVBoxLayout()
    self.cwd_layout = QtGui.QHBoxLayout()
    #the working directory
    self.cwd_layout.addWidget(QtGui.QLabel("Current Working Directory:"))
    self.cwd_text = QtGui.QLineEdit()
    self.cwd_text.setToolTip('The directory where the simulation will be run.')
    self.cwd_text.setReadOnly(True)
    self.cwd_text.setText(os.getcwd())
    self.cwd_button = QtGui.QPushButton("Choose")
    self.cwd_button.setToolTip('Choose directory where the simulation will be run.')
    QtCore.QObject.connect(self.cwd_button, QtCore.SIGNAL("clicked()"), self.clickedCwd)
    self.cwd_layout.addWidget(self.cwd_text)
    self.cwd_layout.addWidget(self.cwd_button)
    #The advancement bar
    self.pb_layout.addWidget(QtGui.QLabel("Progress:"), alignment=Qt.AlignLeft)
    self.pb = QtGui.QProgressBar(self)
    self.pb.hide()
    self.pb_layout.addWidget(self.pb)
    #the log text box
    self.execution_text = QtGui.QTextEdit()
    self.execution_text.setToolTip('The text output from the simulation will show here.  If this is scrolled to the bottom it will automatically scroll as output comes out.')
    self.execution_text.setMinimumHeight(300)
    self.execution_text.setMinimumWidth(800)
    self.execution_text.setFontFamily('Courier')
    self.execution_text.setFontPointSize(10)
    self.execution_text.setReadOnly(True)

    #the bottom of the GUI page
    self.bottom_layout = QtGui.QHBoxLayout()
    #the kill button
    self.kill_button = QtGui.QPushButton("Kill")
    self.kill_button.setToolTip('Stop the currently running simulation')
    self.kill_button.setDisabled(True)
    QtCore.QObject.connect(self.kill_button, QtCore.SIGNAL("clicked()"), self.clickedKill)
    self.bottom_layout.addWidget(self.kill_button, alignment=Qt.AlignHCenter)
    #the log clear button
    self.clear_button = QtGui.QPushButton("Clear Log")
    self.clear_button.setToolTip('Clear the text output from the box above')
    self.clear_button.setDisabled(True)
    QtCore.QObject.connect(self.clear_button, QtCore.SIGNAL("clicked()"), self.clickedClear)
    self.bottom_layout.addWidget(self.clear_button, alignment=Qt.AlignHCenter)
    #the save log button
    self.save_log_button = QtGui.QPushButton("Save Log")
    self.save_log_button.setToolTip('Save the current contents of the log to a file')
    self.save_log_button.setDisabled(True)
    QtCore.QObject.connect(self.save_log_button, QtCore.SIGNAL("clicked()"), self.clickedSaveLog)
    self.bottom_layout.addWidget(self.save_log_button, alignment=Qt.AlignHCenter)
 
    #Placing everything in the main layout and than in the GUI object
    self.main_layout = QtGui.QVBoxLayout()
    self.main_layout.addLayout(self.top_layout)
    self.main_layout.addLayout(self.cwd_layout)
    self.main_layout.addLayout(self.pb_layout)
    self.main_layout.addWidget(self.execution_text)
    self.main_layout.addLayout(self.bottom_layout)
    self.setLayout(self.main_layout)
    
    #call the API
    self.modifyUI()

  def modifyUI(self):
    '''
    This will be called after the interface is completely setup to allow an application to modify this tab
    '''
    pass

  def topLayout(self):
    '''
    overload this method to change the top layout
    '''
    self.top_layout  = QtGui.QGridLayout()
    self.top_layout.setSpacing(15)
    self.top_layout.setColumnStretch (0,1)
    self.top_layout.setColumnStretch (1,1)
    self.embededGrid = {}
    self.embededGrid['up'] = QtGui.QGridLayout()
    self.embededGrid['down']  = QtGui.QGridLayout()
    self.embededGrid['up'].setSpacing(15)
    self.embededGrid['down'].setSpacing(15)
    self.embededGrid['up'].setColumnStretch (0,1)
    self.embededGrid['down'].setColumnStretch (0,1)
    self.embededGrid['up'].setColumnStretch (1,1)
    self.embededGrid['down'].setColumnStretch (1,1)

    #The button activating the run setting windows
    self.runSetButton = QtGui.QPushButton("Advanced Parallel Setting")
    self.runSetButton.setToolTip('Specialize the command line used for parallel and multi-thread')
    QtCore.QObject.connect(self.runSetButton, QtCore.SIGNAL("clicked()"), self.clickedOneRunSetting)
    self.top_layout.addWidget(self.runSetButton, 0, 0)
    #the number of mpi processors
    _labelMpiProc              = QtGui.QLabel('Number of MPI processes')
    self.mpiProcLine           = QtGui.QLineEdit()
    self.embededGrid['up'].addWidget(_labelMpiProc, 0, 0) 
    self.embededGrid['up'].addWidget(self.mpiProcLine, 0, 1)
    self.top_layout.addLayout(self.embededGrid['up'], 1, 0)
    #the number of threads
    _labelMultiThreadsProc     = QtGui.QLabel('Number of threads')
    self.multiThreadsProcLine  = QtGui.QLineEdit()
    self.embededGrid['down'].addWidget(_labelMultiThreadsProc, 0, 0) 
    self.embededGrid['down'].addWidget(self.multiThreadsProcLine, 0, 1)
    self.top_layout.addLayout(self.embededGrid['down'], 2, 0)
    #The run command
    self.run_button = QtGui.QPushButton("Run")
    self.run_button.setToolTip('Will run the executable with the input file described on the Input File tab.')
    QtCore.QObject.connect(self.run_button, QtCore.SIGNAL("clicked()"), self.clickedRun)
    self.top_layout.addWidget(self.run_button, 0, 1)    
    #the CVS flag
    self.postprocessor_csv = QtGui.QCheckBox("Enable Postprocessor CSV Output")
    self.postprocessor_csv.setToolTip('This should be checked if you wish to view postprocessor plots on the Postprocess tab.')
    self.postprocessor_csv.setCheckState(QtCore.Qt.Checked)
    self.top_layout.addWidget(self.postprocessor_csv, 1, 1)        

  def clickedOneRunSetting(self):      
    #setting the dialog box up
    self.oneRunSetting = QtGui.QDialog( parent = self)
    self.oneRunSetting.setModal(True)
    self.oneRunSetting.setSizeGripEnabled (True)
    #labels
    _labelMpiCommand           = QtGui.QLabel('MPI command')
    _labelMultiThreadsCommand  = QtGui.QLabel('Multi-threads command')
    _labelOtherOptionsText     = QtGui.QLabel('Other running options ')
    #text boxes                             
    self.oneRunSetting.mpiCommand           = QtGui.QLineEdit()
    self.oneRunSetting.multiThreadsCommand  = QtGui.QLineEdit()
    self.oneRunSetting.otherOptionsText     = QtGui.QLineEdit()
    #button for accepting setting
    _acceptButton = QtGui.QPushButton("Accept")
    _acceptButton.clicked.connect(lambda:self.oneRunInfoSet('fromDialogBox'))
    self.oneRunInfoSetCopied.connect(self.oneRunSetting.close)
    #button to reset default settings
    _defaultButton = QtGui.QPushButton("Reset default")
    _defaultButton.clicked.connect(lambda:self.oneRunInfoSet('deafaultInDialogBox'))
    #button to discharge any changes
    _cancelButton = QtGui.QPushButton("Reject")
    _cancelButton.clicked.connect(self.oneRunSetting.close)
    #Tool tips
    self.oneRunSetting.mpiCommand.setToolTip('MPI command to be used.')
    self.oneRunSetting.multiThreadsCommand.setToolTip('multi-threading command.')
    self.oneRunSetting.otherOptionsText.setToolTip('Other options to add on the command line.  PETSc options are valid to put here.')
    _acceptButton.setToolTip('Accept displayed values')
    _defaultButton.setToolTip('Brings back default options')
    _cancelButton.setToolTip('Discharge modifications')
    #set text
    if self.oneRunInfoSetSaved:
      self.oneRunInfoSet('fromMain')
    else:
      self.oneRunInfoSet('deafaultInDialogBox')
     #set the dialog box layout
    _grid = QtGui.QGridLayout()
    _grid.setSpacing(10)
     #adding the grid element
    _grid.addWidget(_labelMpiCommand, 0, 0)
    _grid.addWidget(self.oneRunSetting.mpiCommand, 0, 1,1,2)
    _grid.addWidget(_labelMultiThreadsCommand, 2, 0)
    _grid.addWidget(self.oneRunSetting.multiThreadsCommand, 2, 1,1,2)
    _grid.addWidget(_labelOtherOptionsText, 4, 0)
    _grid.addWidget(self.oneRunSetting.otherOptionsText, 4, 1,1,2)
    _grid.addWidget(_acceptButton,5,0)
    _grid.addWidget(_defaultButton,5,1)
    _grid.addWidget(_cancelButton,5,2)
    #show off the beautiful job
    self.oneRunSetting.setLayout(_grid)
    self.oneRunSetting.show()

  #attribute handling the default/copy to from the clickedOneRunSetting dialog box for running setups for each single run
  def oneRunInfoSet(self,_fromWhere): 
    if _fromWhere == 'fromMain':
      self.oneRunSetting.mpiCommand.setText(self.mpiCommand)
      self.oneRunSetting.multiThreadsCommand.setText(self.multiThreadsCommand)
      self.oneRunSetting.otherOptionsText.setText(self.otherOptionsText)
    elif _fromWhere == 'fromDialogBox':
      self.mpiCommand          = self.oneRunSetting.mpiCommand.text()
      self.multiThreadsCommand = self.oneRunSetting.multiThreadsCommand.text()
      self.otherOptionsText    = self.oneRunSetting.otherOptionsText.text()
      self.oneRunInfoSetCopied.emit()
      self.oneRunInfoSetSaved = True
    elif _fromWhere == 'deafaultInDialogBox':
      self.oneRunSetting.mpiCommand.setText(' mpiexec -n ')
      self.oneRunSetting.multiThreadsCommand.setText(' --n-threads=')
      self.oneRunSetting.otherOptionsText.setText('')
    elif _fromWhere == 'deafaultInMain':
      self.mpiCommand           =  ' mpiexec -n '
      self.multiThreadsCommand  = ' --n-threads='
      self.otherOptionsText     = ''

  #the running begin: set up, run, and connecting the end signal
  def clickedRun(self):
    self.isItRunning = True
    num_steps = self.getNumSteps()
    if num_steps:
      self.pb.reset()
      self.pb.setMaximum(num_steps)
      self.pb.show()
      self.pb.setValue(0)
    tmp_file_name = 'peacock_run_tmp.i'
    file = open(tmp_file_name,'w')
    file.write(self.input_file_widget.input_file_textbox.buildInputString())
    file.close()
    command = self.buildCommand(tmp_file_name)
    self.runIt(command)

  #evaluating possible number of steps
  def getNumSteps(self):
    executioner_item = self.input_file_widget.tree_widget.findItems("Executioner", QtCore.Qt.MatchExactly)[0]
    cur_steps = 0
    if executioner_item:
      table_data = executioner_item.table_data
      if 'num_steps' in table_data:
        cur_steps = float(table_data['num_steps'])
      if 'end_time' in table_data and 'dt' in table_data:
        steps = float(table_data['end_time']) / float(table_data['dt'])
        if cur_steps == 0 or steps < cur_steps:
          cur_steps = steps
      for i in range(executioner_item.childCount()):
        child = executioner_item.child(i) 
        if child.text(0) == 'Adaptivity':
          try:
            table_data = child.table_data
            if 'steps' in table_data:
              cur_steps += table_data['steps']
          except:
            pass
    return cur_steps + 2 + 1 # The +2 is for setup steps the +1 is so there is always a bit left...      

  #constructing the command line
  def buildCommand(self,_inFile):
    
    self.multiThreadsProc      = self.multiThreadsProcLine.text()
    self.mpiProc               = self.mpiProcLine.text()
    _command = self.app_path + ' -i ' + _inFile
    if self.mpiProc != '':
      _command = self.mpiCommand + ' ' + self.mpiProc + ' ' + _command
    if self.multiThreadsProc != '':
      _command = _command + self.multiThreadsCommand + self.multiThreadsProc
    _command += ' ' + self.otherOptionsText
    if self.postprocessor_csv.checkState() == Qt.Checked:
      _command += ' Output/postprocessor_csv=true '
    if self.otherOptionsText !='':
      _command += _command
    return _command

  #we are running it for real  
  def runIt(self, command):
    self.execution_text.verticalScrollBar().setValue(self.execution_text.verticalScrollBar().maximum())
    self.execution_text.append('-----\n' + 'Command: ' + '\n' + command + '\n' + '-----\n')
    self.proc = QtCore.QProcess()
    self.connect(self.proc,QtCore.SIGNAL("readyReadStandardOutput()"),self.output)
    self.connect(self.proc,QtCore.SIGNAL("finished(int)"),self.processFinished)
    self.proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    self.run_started.emit()
    self.proc.start(command)
    self.kill_button.setDisabled(False)
    self.clear_button.setDisabled(False)
    self.save_log_button.setDisabled(False)

  #collect the output and connect the advancement monitor for the bar
  def output(self):
    current_output = str(self.proc.readAllStandardOutput())
    self.execution_text.append(current_output.rstrip('\n'))
    self.incrementPB(current_output)

  #monitor the output to signal the advancement
  def incrementPB(self, text):
    split = text.split('\n')
    # List of keywords to increment the PB on
    increment_on = ['Mesh Information','EquationSystems','NL step  0']
    for line in split:
      if 'NL step  0' in line:
        self.timestep_begin.emit()
      if 'Norm of each nonlinear' in line:
        self.timestep_end.emit()
      for keyword in increment_on:
        if keyword in line:  
          self.pb.setValue(self.pb.value()+1)
    
  #broadcast the good news via 'run_stopped.emit' and turn of the streaming to the text box
  def processFinished(self):
    self.kill_button.setDisabled(True)
    self.pb.hide()
    self.run_stopped.emit()
    self.isItRunning = False

  def clickedKill(self):
    msgBox = QMessageBox();
    msgBox.setText("Terminate current run?");
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No);
    msgBox.setDefaultButton(QMessageBox.No);
    ret = msgBox.exec_();
    if ret == QMessageBox.Yes:
      self.killIt()
      
  def killIt(self):
      self.proc.terminate()
      self.processFinished()

  def clickedClear(self):
    msgBox = QMessageBox();
    msgBox.setText("Clear log?");
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No);
    msgBox.setDefaultButton(QMessageBox.No);
    ret = msgBox.exec_();
    if ret == QMessageBox.Yes:
      self.execution_text.setText('')
      if not self.isItRunning:
        self.clear_button.setDisabled(True)
        self.save_log_button.setDisabled(True)

  def clickedSaveLog(self):
    file_name = QtGui.QFileDialog.getSaveFileName(self, "Save Log", "~/", "Log Files (*.log)")
    if file_name != '':
      file = open(file_name,'w')
      file.write(self.execution_text.toPlainText())
      file.close()

  def clickedCwd(self):
    dir_name = QtGui.QFileDialog.getExistingDirectory(self, "Choose CWD")
    if dir_name != '':
      self.cwdText.setText(dir_name)
      os.chdir(dir_name)

  def click_open(self):
    self.cwdText.setText(os.getcwd())

  def click_save(self):
    self.cwdText.setText(os.getcwd())
        


  ''' Return the name to use for this tab '''
  def name(self):
    return 'Execute'
