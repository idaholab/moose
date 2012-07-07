#!/usr/bin/python
from PyQt4 import QtCore, QtGui
from PyQt4.Qt import *
from GenSyntax import *
from ParamTable import *

try:
  _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
  _fromUtf8 = lambda s: s


class CommentEditor(QtGui.QDialog):
  def __init__(self, item, win_parent=None):
    QtGui.QDialog.__init__(self, win_parent)
    self.item = item
    
    self.layout = QtGui.QVBoxLayout()
    self.setLayout(self.layout)
    
    self.edit_box = QtGui.QTextEdit()    
    try:
      self.edit_box.insertPlainText(item.comment)
    except:
      pass
    
    self.layout.addWidget(self.edit_box)

    self.button_layout = QHBoxLayout()
    self.apply_button = QPushButton('Apply')
    self.cancel_button = QPushButton('Cancel')

    self.button_layout.addWidget(self.apply_button)
    self.button_layout.addWidget(self.cancel_button)

    QtCore.QObject.connect(self.apply_button, QtCore.SIGNAL("clicked()"), self.accept_text)
    QtCore.QObject.connect(self.cancel_button, QtCore.SIGNAL("clicked()"), self.reject)

    self.layout.addLayout(self.button_layout)

    self.resize(700,500)

  def accept_text(self):
    self.item.comment = str(self.edit_box.toPlainText())
    self.accept()
