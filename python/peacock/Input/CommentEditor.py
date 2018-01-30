#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QPlainTextEdit, QSizePolicy
from peacock.utils import WidgetUtils
from PyQt5.QtCore import pyqtSignal

class CommentEditor(QWidget):
    textChanged = pyqtSignal()

    def __init__(self, comments=""):
        """
        Just holds a TextEdit and a label
        """
        super(CommentEditor, self).__init__()
        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.setLayout(self.top_layout)
        self.editor = QPlainTextEdit(self)
        self.editor.resize(10, 10)
        self.editor.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        self.editor.setPlainText(comments)
        self.label = WidgetUtils.addLabel(self.top_layout, self, "Comment")
        self.top_layout.addWidget(self.editor)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        self.setMinimumSize(0, 20)
        self.editor.textChanged.connect(self.textChanged)

    def setComments(self, comments):
        self.editor.setPlainText(comments)

    def getComments(self):
        return str(self.editor.toPlainText())
