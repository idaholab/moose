#!/usr/bin/python
from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *
from GenSyntax import *
from ParamTable import *

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s


class OptionsGUI(QtGui.QDialog):
  def __init__(self, main_data, action_syntax, single_item, incoming_data, incoming_param_comments, incoming_comment, already_has_parent_params, type_options, global_params, win_parent=None):
    QtGui.QDialog.__init__(self, win_parent)
#    self.main_ui = QtGui.QWidget(self)
#    self.main_ui.setObjectName(_fromUtf8("Add Subblock"))
    self.layout = QtGui.QVBoxLayout()
    self.setLayout(self.layout)
    self.param_table = ParamTable(main_data, action_syntax, single_item, incoming_data, incoming_param_comments, incoming_comment, self.layout, self, already_has_parent_params, type_options, global_params)
    self.resize(700,500)

  def result(self):
    self.param_table.comment = str(self.param_table.comment_box.toPlainText())
    return self.param_table.result()

  def accept_params(self):
    self.accept()

  def reject_params(self):
    self.reject()
