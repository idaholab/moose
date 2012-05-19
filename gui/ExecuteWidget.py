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

class EmittingStream(QtCore.QObject):
  textWritten = QtCore.pyqtSignal(str)

  def write(self, text):
    self.textWritten.emit(str(text))

class ExecuteWidget(QtGui.QWidget):
  def __init__(self, app_path, input_file_widget, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path = app_path
    self.input_file_widget = input_file_widget
    self.main_layout = QtGui.QVBoxLayout()
    self.setLayout(self.main_layout)

    self.command_layout = QtGui.QHBoxLayout()

    self.mpi_layout = QtGui.QVBoxLayout()
    self.mpi_layout.addWidget(QtGui.QLabel("Number of MPI"))
    self.mpi_text = QtGui.QLineEdit()
    self.mpi_text.setMinimumWidth(10)
    self.mpi_layout.addWidget(self.mpi_text)
    self.command_layout.addLayout(self.mpi_layout)

    self.threads_layout = QtGui.QVBoxLayout()
    self.threads_layout.addWidget(QtGui.QLabel("Number of Threads"))
    self.threads_text = QtGui.QLineEdit()
    self.threads_text.setMinimumWidth(10)
    self.threads_layout.addWidget(self.threads_text)
    self.command_layout.addLayout(self.threads_layout)

    self.other_options_layout = QtGui.QVBoxLayout()
    self.other_options_layout.addWidget(QtGui.QLabel("Other Options"))
    self.other_options_text = QtGui.QLineEdit()
    self.other_options_text.setMinimumWidth(10)
    self.other_options_layout.addWidget(self.other_options_text)
    self.command_layout.addLayout(self.other_options_layout)

    self.run_button = QtGui.QPushButton("Run")
    QtCore.QObject.connect(self.run_button, QtCore.SIGNAL("clicked()"), self.clickedRun)
    self.command_layout.addWidget(self.run_button)
    
    self.execution_text = QtGui.QTextEdit()
    self.execution_text.setMinimumHeight(400)
    self.execution_text.setReadOnly(True)

    
    self.main_layout.addLayout(self.command_layout)
    self.main_layout.addWidget(self.execution_text)
    
  def clickedRun(self):
    tmp_file_name = 'peacock_run_tmp.i'
    file = open(tmp_file_name,'w')
    file.write(self.input_file_widget.buildInputString())
    file.close()
    command = self.app_path + ' -i ' + tmp_file_name

    if self.mpi_text.text() != '':
      command = 'mpiexec -n ' + self.mpi_text.text() + ' ' + command

    if self.threads_text.text() != '':
      command = command + ' --n-threads=' + self.threads_text.text()
      
    command += ' ' + self.other_options_text.text()

    print command
    self.proc = QtCore.QProcess()
    self.connect(self.proc,QtCore.SIGNAL("readyReadStandardOutput()"),self.output)
    self.proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    self.proc.start(command)
    self.proc.waitForFinished()
    self.execution_text.append(str(self.proc.readAllStandardOutput()))

  def output(self):
    self.execution_text.append(str(self.proc.readAllStandardOutput()))
