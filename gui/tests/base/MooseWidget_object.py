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
_test = TestMooseWidget(main=main)
_test.addObject(SubTestMooseWidget(), handle='sub_widget')
_test.addObject(QtGui.QWidget(), handle='non_moose_widget')
_test.setProperty('debug', True)

# Test call to 'object' fails when an invalid parent name is given
def invalidParentObject():
  obj = _test.object('sub_widget', owner='invalid_parent_name')
  result = obj == None
  fail_msg = 'Object is not None'
  return (result, fail_msg)

# Test 'object' pass when a valid parent object name is given
def validParentObject():
  _test.setProperty('debug', True)
  obj = _test.object('sub_sub_widget', owner='sub_widget')
  result = obj.property('handle') == 'sub_sub_widget'
  fail_msg = 'Object unexpected handle'
  return (result, fail_msg)

# Test that 'object' fails when an invalid child object name is given
def invalidObject():
  obj = _test.object('invalid_widget_name')
  result = obj == None
  fail_msg = 'Object is not None'
  return (result, fail_msg)

# Test that 'object' returns a valid object when an valid child object name is given
def validChildObject():
  obj = _test.object('sub_widget')
  result = obj.property('handle') == 'sub_widget'
  fail_msg = 'Object has invalid handle'
  return (result, fail_msg)

# Test that 'object' returns a valid object when an valid grandchild object name is given
def validGrandChildObject():
  obj = _test.object('sub_sub_widget')
  result = obj.property('handle') == 'sub_sub_widget'
  fail_msg = 'Object has invalid handle'
  return (result, fail_msg)

# Test duplicate name error message
def addObjectDuplicateName():
  _test.addObject(QtGui.QWidget(), handle='sub_widget')
  result = _test.testLastErrorMessage('The handle sub_widget already exists')
  fail_msg = 'No expected error'
  return (result, fail_msg)

# Test duplicate objects
def multipleObjects():
  _test.addObject(SubTestMooseWidget(), handle='another_sub_widget')
  objs = _test.object('sub_sub_widget')
  result = all(obj.property('handle') == 'sub_sub_widget' for obj in objs)
  fail_msg = 'Expected widgets not found'
  del _test._objects['another_sub_widget'] # remove this so it doesn't mess up other tests
  return (result, fail_msg)

# Test error for owner != MooseWidget
def errorOwnerNotMooseWidget():
  obj = _test.object('name_does_not_matter', owner='non_moose_widget', error=True)
  result = _test.hasErrorMessage('The owner object non_moose_widget must be a MooseWidget')
  return (result, 'Wrong error')

# Test error for owner != MooseWidget
def errorInvalidOwnerName():
  obj = _test.object('name_does_not_matter', owner='invalid_owner', error=True)
  result = _test.hasErrorMessage('Invalid owner object name invalid_owner when')
  return (result, 'Wrong error')

# Test error object
def errorInvalidWidgetName():
  obj = _test.object('invalid_widget_name', error=True)
  result = _test.testLastErrorMessage('The handle, invalid_widget_name, was not located in the')
  fail_msg = 'No expected error'
  return (result, fail_msg)

# Test error object
def errorInvalidWidgetName():
  obj = _test.object('invalid_widget_name', error=True)
  result = _test.testLastErrorMessage('The handle, invalid_widget_name, was not located in the')
  fail_msg = 'No expected error'
  return (result, fail_msg)

def errorMultipleWidgets():
  _test.addObject(SubTestMooseWidget(), handle='another_sub_widget')
  objs = _test.object('sub_sub_widget', error=True)
  result = _test.hasErrorMessage('Multiple handles located with the name sub_sub')
  del _test._objects['another_sub_widget'] # remove this so it doesn't mess up other tests
  return (result, 'No expected error')
