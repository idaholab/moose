import os, sys, re
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Widget for executing and displaying MOOSE executable results
class ExecuteWidgetConsole(QtGui.QWidget, MooseWidget):

# public:
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Create progress bar and console window
    #self.addObject(QtGui.QProgressBar(self), handle='ExecutionProgress', label='Progress:')
    self._console = self.addObject(QtGui.QTextEdit(), handle='Console')
   # self._console.verticalScrollBar().setValue(self._console.verticalScrollBar().maximum())



    self.setup()

  ##
  # Collect the output and connect the advancement monitor for the bar
  def updateConsole(self, proc):

    while proc.canReadLine():
      cache = re.sub(r'\[(\d\d)m(.*)(\[39m)', self.testFunc, proc.readLine().data().rstrip('\n'))
      self._console.append('<pre style="display:inline">' + cache + '</pre>')

  def testFunc(self, match):
    clr = None
    c = int(match.group(1))
    if c == 32:
      clr = 'green'


    if clr:
      return '<span style="color:green">' + match.group(2) + '</span>'
    else:
      return match.group(0)



#define BLACK    "\33[30m"
#define RED      "\33[31m"
#define GREEN    "\33[32m"
#define YELLOW   "\33[33m"
#define BLUE     "\33[34m"
#define MAGENTA  "\33[35m"
#define CYAN     "\33[36m"
#define WHITE    "\33[37m"
#define DEFAULT  "\33[39m"

#private:

  ##
  # Setup method for the Console display
  def _setupConsole(self, q_object):
    q_object.setMinimumHeight(300)
    q_object.setMinimumWidth(800)
    q_object.setFontFamily('Courier')
    q_object.setFontPointSize(10)

    q_object.setReadOnly(True)
    q_object.setUndoRedoEnabled(False)
    q_object.setFrameStyle(QtGui.QFrame.NoFrame)
    q_object.setStyleSheet('color:white;background-color:black;')
    text_format = QtGui.QTextCharFormat()
    text_format.setFontFixedPitch(True)
    q_object.setCurrentCharFormat(text_format)

    #q_object.verticalScrollBar().setValue(q_object.verticalScrollBar().maximum())
