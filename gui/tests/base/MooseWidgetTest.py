#!/usr/bin/python
import os, sys
from PySide import QtGui

# Import Peacock modules
from src import *
from src.utils import TestObject

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

# Create a global object for testing
#if ('app' not in globals()) or (not app.isinstance(QtGui.QApplication)):
#  app  = QtGui.QApplication(sys.argv)


class MooseWidgetTest(TestObject):
  def __init__(self, **kwargs):
    TestObject.__init__(self, **kwargs)

    main = QtGui.QMainWindow()
    self.test = TestMooseWidget(main=main)
    self.test.addObject(SubTestMooseWidget(), handle='sub_widget')
    self.test.addObject(QtGui.QWidget(), handle='non_moose_widget')
    self.test.setProperty('debug', True)

  # Test call to 'object' fails when an invalid parent name is given
  def testInvalidParentObject(self):
    obj = self.test.object('sub_widget', owner='invalid_parent_name')
    result = obj == None
    fail_msg = 'Object is not None'
    return (result, fail_msg)

  # Test 'object' pass when a valid parent object name is given
  def testValidParentObject(self):
    self.test.setProperty('debug', True)
    obj = self.test.object('sub_sub_widget', owner='sub_widget')
    result = obj.property('handle') == 'sub_sub_widget'
    fail_msg = 'Object unexpected handle'

    return (result, fail_msg)

  # Test that 'object' fails when an invalid child object name is given
  def testInvalidObject(self):
    obj = self.test.object('invalid_widget_name')
    result = obj == None
    fail_msg = 'Object is not None'

    return (result, fail_msg)

  # Test that 'object' returns a valid object when an valid child object name is given
  def testValidChildObject(self):
    obj = self.test.object('sub_widget')
    result = obj.property('handle') == 'sub_widget'
    fail_msg = 'Object has invalid handle'

    return (result, fail_msg)

  # Test that 'object' returns a valid object when an valid grandchild object name is given
  def testValidGrandChildObject(self):
    obj = self.test.object('sub_sub_widget')
    result = obj.property('handle') == 'sub_sub_widget'
    fail_msg = 'Object has invalid handle'

    return (result, fail_msg)

  # Test duplicate name error message
  def testAddObjectDuplicateName(self):
    self.test.addObject(QtGui.QWidget(), handle='sub_widget')
    result = self.test.testLastErrorMessage('The handle sub_widget already exists')
    fail_msg = 'No expected error'

    return (result, fail_msg)

  # Test duplicate objects
  def testMultipleObjects(self):
    self.test.addObject(SubTestMooseWidget(), handle='another_sub_widget')
   # objs = self.test.object('sub_sub_widget')
   # result = all(obj.property('handle') == 'sub_sub_widget' for obj in objs)
   # fail_msg = 'Expected widgets not found'
   # del self.test._objects['another_sub_widget'] # remove this so it doesn't mess up other tests
    return (False, 'debugging')
    return (result, fail_msg)

  # Test error for owner != MooseWidget
  def testErrorOwnerNotMooseWidget(self):
    obj = self.test.object('name_does_not_matter', owner='non_moose_widget', error=True)
    result = self.test.testLastErrorMessage('The owner object non_moose_widget must be a MooseWidget')

    return (result, 'Wrong error')

  # Test error for owner != MooseWidget
  def testErrorInvalidOwnerName(self):
    obj = self.test.object('name_does_not_matter', owner='invalid_owner', error=True)
    result = self.test.testLastErrorMessage('Invalid owner object name invalid_owner when')

    return (result, 'Wrong error')

  # Test error object
  def testErrorInvalidWidgetName(self):
    obj = self.test.object('invalid_widget_name', error=True)
    result = self.test.testLastErrorMessage('The handle, invalid_widget_name, was not located in the')
    fail_msg = 'No expected error'

    return (result, fail_msg)

  # Test error object
  def testErrorInvalidWidgetName(self):
    obj = self.test.object('invalid_widget_name', error=True)
    result = self.test.testLastErrorMessage('The handle, invalid_widget_name, was not located in the')
    fail_msg = 'No expected error'

    return (result, fail_msg)

  def testErrorMultipleWidgets(self):
    #self.test.addObject(SubTestMooseWidget(), handle='another_sub_widget')
    #objs = self.test.object('sub_sub_widget', error=True)
    #result = self.test.testLastErrorMessage('Multiple handles located with the name sub_sub')
    #del self.test._objects['another_sub_widget'] # remove this so it doesn't mess up other tests
    return (False, 'Debugging')
    return (result, 'No expected error')
