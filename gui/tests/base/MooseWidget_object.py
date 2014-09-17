#!/usr/bin/python
########################################
# Tests for MooseObject::object method #
########################################
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
#test.addObject(SubTestMooseWidget(), handle='another_sub_widget')
test.setProperty('debug', True)

# Test call to 'object' fails when an invalid parent name is given
def invalidParentObject():
  obj = test.object('sub_widget', parent='invalid_parent_name')
  result = obj == None
  fail_msg = 'Object is not None'
  return (result, fail_msg)

# Test 'object' pass when a valid parent object name is given
def validParentObject():
  test.setProperty('debug', True)
  obj = test.object('sub_sub_widget', parent='sub_widget')
  result = obj.property('handle') == 'sub_sub_widget'
  fail_msg = 'Object unexpected handle'
  return (result, fail_msg)

# Test that 'object' fails when an invalid child object name is given
def invalidObject():
  obj = test.object('invalid_widget_name')
  result = obj == None
  fail_msg = 'Object is not None'
  return (result, fail_msg)

# Test that 'object' returns a valid object when an valid child object name is given
def validChildObject():
  obj = test.object('sub_widget')
  result = obj.property('handle') == 'sub_widget'
  fail_msg = 'Object has invalid handle'
  return (result, fail_msg)

# Test that 'object' returns a valid object when an valid grandchild object name is given
def validGrandChildObject():
  obj = test.object('sub_sub_widget')
  result = obj.property('handle') == 'sub_sub_widget'
  fail_msg = 'Object has invalid handle'
  return (result, fail_msg)
