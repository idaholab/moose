import os, sys, re
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Widget for displaying executable results
class ExecuteConsole(QtGui.QWidget, MooseWidget):

# public:
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Create console window, storing the object as a member variable for easy access
    self._progress = self.addObject(QtGui.QProgressBar(), handle='Progress')
    self._console = self.addObject(QtGui.QTextEdit(), handle='Console')

    # Execute the setup functions
    self.setup()

  ##
  # Update the text in the console text edit object (public)
  # @param process The QProcess to capture lines from (see ExecuteWidget)
  def updateConsole(self, process):

    while process.canReadLine():
      line = process.readLine().data().rstrip('\n')
      text = re.sub(r'(.)\[(\d\d)m(.*)(.)(\[39m)', self._colorText, line)
      self._console.append('<span style="white-space:pre;font-family:courier">' + text + '</span>')

      match = re.search(r'Time\sStep\s*([0-9]{1,})', line)
      if match:
        self._progress.setValue(float(match.group(1)))

  ##
  # Reset progress bar
  def resetProgress(self):
    num_steps = self.pull('NumSteps')
    if num_steps:
      self._progress.setRange(0, num_steps)
    else:
      self._progress.reset()

  ##
  # A method for coloring Console text via html (private)
  # @param match The re match, see updateConsole method
  def _colorText(self, match):

    # Define the html color from the bash color codes
    html_color = None
    color_code = int(match.group(2))
    if color_code == 30:
      html_color = 'black'
    elif color_code == 31:
      html_color = 'red'
    elif color_code == 32:
      html_color = 'green'
    elif color_code == 33:
      html_color = 'yellow'
    elif color_code == 34:
      html_color = 'blue'
    elif color_code == 35:
      html_color = 'magenta'
    elif color_code == 36:
      html_color = 'cyan'
    elif color_code == 37:
      html_color = 'white'

    # Apply the color, otherwise just return the complete match
    if html_color:
      return '<span style="color:' + html_color+ ';">' + match.group(3) + '</span>'
    else:
      return match.group(0)

  ##
  # Setup method for the progress bar
  def _setupProgress(self, q_object):
    self.resetProgress()

  ##
  # Setup method for the Console display
  def _setupConsole(self, q_object):
    q_object.setMinimumHeight(300)
    q_object.setMinimumWidth(800)
    q_object.setReadOnly(True)
    q_object.setUndoRedoEnabled(False)
    q_object.setFrameStyle(QtGui.QFrame.NoFrame)
    q_object.setStyleSheet('color:white;background-color:black;white-space:pre;font-family:courier;font:10pt')
