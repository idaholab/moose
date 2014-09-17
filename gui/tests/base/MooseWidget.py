#!/usr/bin/python
import os, sys
from PySide import QtGui

# Import Peacock modules
from src import *

# Create a subclass
class SubTestMooseWidget(QtGui.QWidget, base.MooseWidget):
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    base.MooseWidget.__init__(self, **kwargs)
    self.addObject(QtGui.QWidget(), handle='sub_sub_widget')

# Create a MooseWidget object for testing
class TestMooseWidget(QtGui.QWidget, base.MooseWidget):
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    base.MooseWidget.__init__(self, **kwargs)

# Create
app  = QtGui.QApplication(sys.argv)
main = QtGui.QMainWindow()
test = TestMooseWidget(main=main)
test.addObject(SubTestMooseWidget(), handle='sub_widget')

# Test locating of a direct child
def hasLocalObject():
  obj = test.object('sub_widget')
  result = test.hasObject('sub_widget')
  fail_msg = 'Failed local find'
  return (result, fail_msg)
