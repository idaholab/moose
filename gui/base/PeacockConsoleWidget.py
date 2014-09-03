# Python modules
import sys
from code import InteractiveConsole
from StringIO import StringIO

# Qt modules
from PySide import QtCore, QtGui

# Peacock modules
from base import *

##
# An interactive python interpreter class
#
# This class inherits from InteractiveConsole and provides the necessary
# functionality for capturing the output stream from the interactive
# console object.
#
# Note, ideally this would include multiple inheritance and be included
# in PeacockConsoleWidget, but due to InteractiveConsole not using the
# new object syntax multiple inhertence support is limited.
class PeacockInteractiveConsoleHelper(InteractiveConsole):

  ##
  # Class constructor
  # @param write_function A function that writes to the desired location
  # @param locals The local namespace to import
  def __init__(self, write_function, namespace_vars=None, **kwargs):
    InteractiveConsole.__init__(self, namespace_vars)
    self._writeFunction = write_function

  ##
  # Overload the InteractiveConsole::write method
  # @param data The data captured from evaluating the code
  #
  # This method strips extra new lines and calls the writeFunction
  def write(self, data):
    if data:
      if data[-1] == '\n':
        data = data[:-1]
        self._writeFunction(data)

  ##
  # Runs the supplied source through the interactive interpreter
  # Overloads the InteractiveConsole::runsource method
  # @param source The code to execute
  # @return True if additional code is required to execute
  def runsource(self, source, filename="<input>", symbol="single"):

      more = False
      old_stdout = sys.stdout
      old_stderr = sys.stderr
      sys.stdout = sys.stderr = collector = StringIO()
      try:
        more = InteractiveConsole.runsource(self, source, filename, symbol)
      finally:
        if sys.stdout is collector:
          sys.stdout = old_stdout
        if sys.stderr is collector:
          sys.stderr = old_stderr
      self.write(collector.getvalue())
      return more

##
# An interactive console widget for debugging
class PeacockConsoleWidget(QtGui.QWidget, MooseWidget):

  ##
  # Constructor
  # @param namespace_vars The variables to import (e.g., locals() or globals())
  # @param kwargs Key-value pairs that are passed to the MooseWidget
  def __init__(self, namespace_vars=None, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Add the GUI objects
    self.addObject(QtGui.QVBoxLayout(), handle='ConsoleLayout')
    output_object = self.addObject(QtGui.QPlainTextEdit(), handle='ConsoleOutput', parent='ConsoleLayout')
    self.addObject(QtGui.QHBoxLayout(), handle='ConsolePromptInputLayout', parent='ConsoleLayout')
    self.addObject(QtGui.QLineEdit(), handle='ConsolePrompt', parent='ConsolePromptInputLayout')
    self.addObject(QtGui.QLineEdit(), handle='ConsoleInput', parent='ConsolePromptInputLayout')

    # Create the helper class for performing console operations
    self._console = PeacockInteractiveConsoleHelper(output_object.appendPlainText, namespace_vars)

    # Define short-cut to the 'ConsoleInput'; this is used in an eventFilter which is continuously being
    # called. To avoid searching for the object over and over, just store a local variable
    self._input = self.object('ConsoleInput')

    # Define the storage for the command history and current history location
    self._history = []
    self._history_position = 0

    # Call the setup methods
    self.setup()

  ##
  # Event filter for 'ConsoleInput'; this handles updating the command history
  # @param q_object The object creating the event
  # @param event The QEvent object
  def eventFilter(self, q_object, event):
    if event.type() == QtCore.QEvent.KeyPress:
      if event.key() == QtCore.Qt.Key_Up:
        h = self._getHistory(-1)
        self._input.setText(h)
      elif event.key() == QtCore.Qt.Key_Down:
        h = self._getHistory(1)
        self._input.setText(h)
    return False

  ##
  # Retrieve the command history
  # @param offset The direction to migrate in the current command history
  # @return The desired command from the history
  def _getHistory(self, offset):

    # Update the history location
    self._history_position += offset

    # Do nothing if at the beginning
    if self._history_position < 0:
      self._history_position = 0
      return ''

    # Do nothing in at the end
    elif self._history_position >= len(self._history):
      self._history_position = len(self._history) - 1
      return ''

    # Return the command from the history storage
    else:
      return self._history[self._history_position]

  ##
  # Setup method for 'ConsoleLayout'
  def _setupConsoleLayout(self, q_object):
    q_object.setSpacing(0)

  ##
  # Setup method for 'ConsoleOutput'
  def _setupConsoleOutput(self, q_object):
    q_object.setReadOnly(True)
    q_object.setUndoRedoEnabled(False)
    q_object.setMaximumBlockCount(5000)
    q_object.setFrameStyle(QtGui.QFrame.NoFrame)
    q_object.setStyleSheet('color:white;background-color:black;')
    text_format = QtGui.QTextCharFormat()
    text_format.setFontFixedPitch(True)
    q_object.setCurrentCharFormat(text_format)

  ##
  # Setup method for 'ConsolePrompt'
  def _setupConsolePrompt(self, q_object):
    q_object.setFrame(False)
    q_object.setStyleSheet('color:white;background-color:black;')
    q_object.setFixedWidth(30)
    q_object.setReadOnly(True)
    q_object.setText('>>> ')

  ##
  # Setup method for 'ConsoleInput'
  def _setupConsoleInput(self, q_object):
    q_object.setFrame(False)
    q_object.setStyleSheet('color:white;background-color:black;')
    q_object.returnPressed.connect(lambda: self._callbackConsoleInput(q_object))
    q_object.installEventFilter(self)

  ##
  # Callback for 'ConsoleInput' that performs evaluation of command
  def _callbackConsoleInput(self, q_object):

    # Short-cuts to Console related objects needed in this callback
    output_object = self.object('ConsoleOutput')
    prompt_object = self.object('ConsolePrompt')

    # Extract the current text from the input widget and update the history
    current_text = q_object.text()
    if current_text:
      self._history.append(current_text)
      self._history_position += 1

    # Clear the command
    q_object.setText('')

    # Pass the command to the output for display
    output_object.appendPlainText(prompt_object.text() + current_text)

    # Run the actual command in the interpreter
    more = self._console.push(current_text)

    # If more data is needed for the current command update the prompt
    if more:
      prompt_object.setText('... ')
    else:
      prompt_object.setText('>>> ')
