#!/usr/bin/python

#  Copyright (c) 2009-2010, Cloud Matrix Pty. Ltd.
#  All rights reserved; available under the terms of the BSD License.
"""
PySideKick.Console:  a simple embeddable python shell
=====================================================


This module provides the call QPythonConsole, a python shell that can be
embedded in your GUI.

"""

import sys
from code import InteractiveConsole as _InteractiveConsole

#from PySideKick import QtCore, QtGui
from PySide import QtCore, QtGui


#import StringIO
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO


class _QPythonConsoleInterpreter(_InteractiveConsole):
    """InteractiveConsole subclass that sends all output to the GUI."""

    def __init__(self,ui,locals=None):
        _InteractiveConsole.__init__(self,locals)
        self.ui = ui

    def write(self,data):
        if data:
            if data[-1] == "\n":
                data = data[:-1]
            self.ui.output.appendPlainText(data)

    def runsource(self,source,filename="<input>",symbol="single"):
        old_stdout = sys.stdout
        old_stderr = sys.stderr
        sys.stdout = sys.stderr = collector = StringIO()
        try:
            more = _InteractiveConsole.runsource(self,source,filename,symbol)
        finally:
            if sys.stdout is collector:
                sys.stdout = old_stdout

            if sys.stderr is collector:
                sys.stderr = old_stderr
        self.write(collector.getvalue())
        return more


class _QPythonConsoleUI(object):
    """UI layout container for QPythonConsole."""
    def __init__(self,parent):
        if parent.layout() is None:
            parent.setLayout(QtGui.QHBoxLayout())
        layout = QtGui.QVBoxLayout()
        layout.setSpacing(0)
        #  Output console:  a fixed-pitch-font text area.
        self.output = QtGui.QPlainTextEdit(parent)
        self.output.setReadOnly(True)
        self.output.setUndoRedoEnabled(False)
        self.output.setMaximumBlockCount(5000)
        self.output.setFrameStyle(QtGui.QFrame.NoFrame)

        fmt = QtGui.QTextCharFormat()
        fmt.setFontFixedPitch(True)
        self.output.setCurrentCharFormat(fmt)
        layout.addWidget(self.output)
        parent.layout().addLayout(layout)
        #  Input console, a prompt displated next to a lineedit
        layout2 = QtGui.QHBoxLayout()
        self.prompt = QtGui.QLabel(parent)
        self.prompt.setFrameStyle(QtGui.QFrame.NoFrame)
        self.prompt.setStyleSheet("background-color:white;");


        self.prompt.setText(">>> ")
        layout2.addWidget(self.prompt)


        self.input = QtGui.QLineEdit(parent)
        self.input.setFrame(False)


        layout2.addWidget(self.input)
        layout.addLayout(layout2)




class QPythonConsole(QtGui.QWidget):
    """A simple python console to embed in your GUI.

    This widget provides a simple interactive python console that you can
    embed in your GUI (e.g. for debugging purposes).  Use it like so:

        self.debug_window.layout().addWidget(QPythonConsole())

    You can customize the variables that are available in the shell by
    passing a dict as the "locals" argument.
    """

    def __init__(self,parent=None,locals=None):
        super(QPythonConsole,self).__init__(parent)
        self.ui = _QPythonConsoleUI(self)
        self.interpreter = _QPythonConsoleInterpreter(self.ui,locals)
        self.ui.input.returnPressed.connect(self._on_enter_line)
        self.ui.input.installEventFilter(self)
        self.history = []
        self.history_pos = 0

    def _on_enter_line(self):
        line = self.ui.input.text()
        self.ui.input.setText("")
        self.interpreter.write(self.ui.prompt.text() + line)
        more = self.interpreter.push(line)
        if line:
            self.history.append(line)
            self.history_pos = len(self.history)
            while len(self.history) > 100:
                self.history = self.history[1:]
                self.history_pos -= 1
        if more:
            self.ui.prompt.setText("... ")
        else:
            self.ui.prompt.setText(">>> ")

    def eventFilter(self,obj,event):
        if event.type() == QtCore.QEvent.KeyPress:
            if event.key() == QtCore.Qt.Key_Up:
                self.go_history(-1)
            elif event.key() == QtCore.Qt.Key_Down:
                self.go_history(1)
        return False

    def go_history(self,offset):
        if offset < 0:
            self.history_pos = max(0,self.history_pos + offset)
        elif offset > 0:
            self.history_pos = min(len(self.history),self.history_pos + offset)
        try:
            line = self.history[self.history_pos]
        except IndexError:
            line = ""
        self.ui.input.setText(line)


if __name__ == "__main__":
    import sys, os
    app = QtGui.QApplication(sys.argv)
    win = QtGui.QMainWindow()
    win.setCentralWidget(QPythonConsole())
    win.show()
    app.exec_()
