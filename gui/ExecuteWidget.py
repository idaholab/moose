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
  def __init__(self, app_path, input_file_widget, qt_app, win_parent=None):
    QtGui.QWidget.__init__(self, win_parent)
    self.app_path = app_path
    self.input_file_widget = input_file_widget
    self.qt_app = qt_app
    self.main_layout = QtGui.QVBoxLayout()
    self.setLayout(self.main_layout)

    self.command_layout = QtGui.QHBoxLayout()
    self.button_layout = QtGui.QHBoxLayout()

    self.mpi_layout = QtGui.QVBoxLayout()
    self.mpi_layout.addWidget(QtGui.QLabel("Number of MPI"), alignment=Qt.AlignHCenter)
    self.mpi_text = QtGui.QLineEdit()
    self.mpi_text.setMinimumWidth(10)
    self.mpi_text.setMaximumWidth(30)
    self.mpi_layout.addWidget(self.mpi_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.mpi_layout)

    self.threads_layout = QtGui.QVBoxLayout()
    self.threads_layout.addWidget(QtGui.QLabel("Number of Threads"), alignment=Qt.AlignHCenter)
    self.threads_text = QtGui.QLineEdit()
    self.threads_text.setMinimumWidth(10)
    self.threads_text.setMaximumWidth(30)
    self.threads_layout.addWidget(self.threads_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.threads_layout)

    self.other_options_layout = QtGui.QVBoxLayout()
    self.other_options_layout.addWidget(QtGui.QLabel("Other Options"), alignment=Qt.AlignHCenter)
    self.other_options_text = QtGui.QLineEdit()
    self.other_options_text.setMinimumWidth(300)
    self.other_options_text.setMaximumWidth(300)
    self.other_options_layout.addWidget(self.other_options_text, alignment=Qt.AlignHCenter)
    self.command_layout.addLayout(self.other_options_layout)

    self.run_button = QtGui.QPushButton("Run")
    QtCore.QObject.connect(self.run_button, QtCore.SIGNAL("clicked()"), self.clickedRun)
    self.command_layout.addWidget(self.run_button, alignment=Qt.AlignHCenter)
    
    self.execution_text = QtGui.QTextEdit()
    self.execution_text.setMinimumHeight(400)
    self.execution_text.setMinimumWidth(800)
    self.execution_text.setFontFamily('Courier')
    self.execution_text.setFontPointSize(10)
    self.execution_text.setReadOnly(True)
    
    self.kill_button = QtGui.QPushButton("Kill")
    self.kill_button.setDisabled(True)
    QtCore.QObject.connect(self.kill_button, QtCore.SIGNAL("clicked()"), self.clickedKill)
    self.button_layout.addWidget(self.kill_button, alignment=Qt.AlignHCenter)

    self.clear_button = QtGui.QPushButton("Clear Log")
    self.clear_button.setDisabled(True)
    QtCore.QObject.connect(self.clear_button, QtCore.SIGNAL("clicked()"), self.clickedClear)
    self.button_layout.addWidget(self.clear_button, alignment=Qt.AlignHCenter)

    self.save_log_button = QtGui.QPushButton("Save Log")
    self.save_log_button.setDisabled(True)
    QtCore.QObject.connect(self.save_log_button, QtCore.SIGNAL("clicked()"), self.clickedSaveLog)
    self.button_layout.addWidget(self.save_log_button, alignment=Qt.AlignHCenter)

    self.main_layout.addLayout(self.command_layout)
    self.main_layout.addWidget(self.execution_text)
    self.main_layout.addLayout(self.button_layout)
    
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

    self.execution_text.append('-----\n' + command + '\n')
    self.proc = QtCore.QProcess()
    self.connect(self.proc,QtCore.SIGNAL("readyReadStandardOutput()"),self.output)
    self.proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    self.proc.start(command)
    self.kill_button.setDisabled(False)
    self.clear_button.setDisabled(False)
    self.save_log_button.setDisabled(False)
    while self.proc.state() != QtCore.QProcess.NotRunning:
      self.qt_app.processEvents()
    self.kill_button.setDisabled(True)

  def output(self):
    self.execution_text.append(str(self.proc.readAllStandardOutput()).rstrip('\n'))

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
