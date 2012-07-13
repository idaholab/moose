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
  # These are the signals this object can emit:
  run_started = QtCore.pyqtSignal()
  run_stopped = QtCore.pyqtSignal()
  timestep_begin = QtCore.pyqtSignal()
  timestep_end = QtCore.pyqtSignal()
  def __init__(self, app_path, input_file_widget, qt_app, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path = app_path
    self.input_file_widget = input_file_widget
    self.qt_app = qt_app
    self.main_layout = QtGui.QVBoxLayout()
    self.setLayout(self.main_layout)

    self.command_layout = QtGui.QHBoxLayout()
    self.cwd_layout = QtGui.QHBoxLayout()
    self.pb_layout = QtGui.QHBoxLayout()
    self.button_layout = QtGui.QHBoxLayout()

    self.mpi_layout = QtGui.QVBoxLayout()
    self.mpi_layout.addWidget(QtGui.QLabel("Number of MPI"), alignment=Qt.AlignHCenter)
    self.mpi_text = QtGui.QLineEdit()
    self.mpi_text.setToolTip('Number of MPI processes.  If left blank then the run will be serial.')
    self.mpi_text.setMinimumWidth(10)
    self.mpi_text.setMaximumWidth(30)
    self.mpi_layout.addWidget(self.mpi_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.mpi_layout)

    self.threads_layout = QtGui.QVBoxLayout()
    self.threads_layout.addWidget(QtGui.QLabel("Number of Threads"), alignment=Qt.AlignHCenter)
    self.threads_text = QtGui.QLineEdit()
    self.threads_text.setToolTip('Number of threads for shared memory parallelism.  If left blank then no threading will be used.')
    self.threads_text.setMinimumWidth(10)
    self.threads_text.setMaximumWidth(30)
    self.threads_layout.addWidget(self.threads_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.threads_layout)

    self.other_options_layout = QtGui.QVBoxLayout()
    self.other_options_layout.addWidget(QtGui.QLabel("Other Options"), alignment=Qt.AlignHCenter)
    self.other_options_text = QtGui.QLineEdit()
    self.other_options_text.setToolTip('Other options to add on the commandline.  PETSc options are valid to put here')
    self.other_options_text.setMinimumWidth(300)
    self.other_options_text.setMaximumWidth(300)
    self.other_options_layout.addWidget(self.other_options_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.other_options_layout)

    self.run_button = QtGui.QPushButton("Run")
    self.run_button.setToolTip('Will run the executable with the input file described on the Input File tab.')
    QtCore.QObject.connect(self.run_button, QtCore.SIGNAL("clicked()"), self.clickedRun)
    self.command_layout.addWidget(self.run_button, alignment=Qt.AlignHCenter)

    self.postprocessor_csv = QtGui.QCheckBox("Enable Postprocessor CSV Output")
    self.postprocessor_csv.setToolTip('This should be checked if you wish to view postprocessor plots on the Postprocess tab.')
    self.postprocessor_csv.setCheckState(QtCore.Qt.Checked)
    
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

    self.pb_layout.addWidget(QtGui.QLabel("Progress:"), alignment=Qt.AlignLeft)
    self.pb = QtGui.QProgressBar(self)
    self.pb.hide()
    self.pb_layout.addWidget(self.pb)
    
    self.execution_text = QtGui.QTextEdit()
    self.execution_text.setToolTip('The text output from the simulation will show here.  If this is scrolled to the bottom it will automatically scroll as output comes out.')
    self.execution_text.setMinimumHeight(400)
    self.execution_text.setMinimumWidth(800)
    self.execution_text.setFontFamily('Courier')
    self.execution_text.setFontPointSize(10)
    self.execution_text.setReadOnly(True)
    
    self.kill_button = QtGui.QPushButton("Kill")
    self.kill_button.setToolTip('Stop the currently running simulation')
    self.kill_button.setDisabled(True)
    QtCore.QObject.connect(self.kill_button, QtCore.SIGNAL("clicked()"), self.clickedKill)
    self.button_layout.addWidget(self.kill_button, alignment=Qt.AlignHCenter)

    self.clear_button = QtGui.QPushButton("Clear Log")
    self.clear_button.setToolTip('Clear the text output from the box above')
    self.clear_button.setDisabled(True)
    QtCore.QObject.connect(self.clear_button, QtCore.SIGNAL("clicked()"), self.clickedClear)
    self.button_layout.addWidget(self.clear_button, alignment=Qt.AlignHCenter)

    self.save_log_button = QtGui.QPushButton("Save Log")
    self.save_log_button.setToolTip('Save the current contents of the log to a file')
    self.save_log_button.setDisabled(True)
    QtCore.QObject.connect(self.save_log_button, QtCore.SIGNAL("clicked()"), self.clickedSaveLog)
    self.button_layout.addWidget(self.save_log_button, alignment=Qt.AlignHCenter)

    self.main_layout.addLayout(self.command_layout)
    self.main_layout.addLayout(self.cwd_layout)
    self.main_layout.addWidget(self.postprocessor_csv, alignment=Qt.AlignHCenter)
    self.main_layout.addLayout(self.pb_layout)
    self.main_layout.addWidget(self.execution_text)
    self.main_layout.addLayout(self.button_layout)

    # These change the CWD... so let's connect to them
    QtCore.QObject.connect(self.input_file_widget.buttonOpen, QtCore.SIGNAL("clicked()"), self.click_open)
    QtCore.QObject.connect(self.input_file_widget.buttonSave, QtCore.SIGNAL("clicked()"), self.click_save)

    
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
    
  def clickedRun(self):
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
    command = self.app_path + ' -i ' + tmp_file_name

    if self.mpi_text.text() != '':
      command = 'mpiexec -n ' + self.mpi_text.text() + ' ' + command

    if self.threads_text.text() != '':
      command = command + ' --n-threads=' + self.threads_text.text()
      
    command += ' ' + self.other_options_text.text()

    if self.postprocessor_csv.checkState() == Qt.Checked:
      command += ' Output/postprocessor_csv=true '

    self.execution_text.verticalScrollBar().setValue(self.execution_text.verticalScrollBar().maximum())

    self.execution_text.append('-----\n' + command + '\n')
    self.proc = QtCore.QProcess()
    self.connect(self.proc,QtCore.SIGNAL("readyReadStandardOutput()"),self.output)
    self.connect(self.proc,QtCore.SIGNAL("finished(int)"),self.processFinished)
    self.proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    self.run_started.emit()
    self.proc.start(command)
    self.kill_button.setDisabled(False)
    self.clear_button.setDisabled(False)
    self.save_log_button.setDisabled(False)


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
    
  def output(self):
    current_output = str(self.proc.readAllStandardOutput())
    self.execution_text.append(current_output.rstrip('\n'))
    self.incrementPB(current_output)

  def processFinished(self):
    self.kill_button.setDisabled(True)
    self.pb.hide()
    self.run_stopped.emit()

  def clickedKill(self):
    msgBox = QMessageBox();
    msgBox.setText("Terminate current run?");
#    msgBox.setInformativeText("Really terminate current run?");
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No);
    msgBox.setDefaultButton(QMessageBox.No);
    ret = msgBox.exec_();
    if ret == QMessageBox.Yes:
      self.proc.terminate()

  def clickedClear(self):
    msgBox = QMessageBox();
    msgBox.setText("Clear log?");
#    msgBox.setInformativeText("Really terminate current run?");
    msgBox.setStandardButtons(QMessageBox.Yes | QMessageBox.No);
    msgBox.setDefaultButton(QMessageBox.No);
    ret = msgBox.exec_();
    if ret == QMessageBox.Yes:
      self.execution_text.setText('')
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
      self.cwd_text.setText(dir_name)
      os.chdir(dir_name)

  def click_open(self):
    self.cwd_text.setText(os.getcwd())

  def click_save(self):
    self.cwd_text.setText(os.getcwd())
