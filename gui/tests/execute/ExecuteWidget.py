#!/usr/bin/python
import os, sys
from PySide import QtGui

import src.execute
from src.utils import TestObject

# Create the application
main = QtGui.QMainWindow()
test = src.execute.ExecuteWidget(main=main, testing=True, debug=False, wait=True)

class ExecuteWidget(TestObject):
  def __init__(self, **kwargs):
    TestObject.__init__(self, **kwargs)
    self.test_app = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'test_app')

  # Clean-up the objects created for this test
  def __del__(self):
    del main, test

  # Test executing a command
  def testRun(self):
    test.object('Executable').setText(self.test_app)
    run = test.callback('Run')
    test.clear() # clear the console output
    run()
    result = test.object('Console').toPlainText() == 'Testing'
    test.clear() # clear the console output
    return (result, 'Failed to run test application')

  # Test executing a command with an optional flag
  def testRunWithFlag(self):
    test.object('Arguments').setText('--flag')
    test.object('Executable').setText(self.test_app)
    run = test.callback('Run')
    test.clear() # clear the console output
    run()
    result = test.object('Console').toPlainText() == 'Testing with flag'
    test.clear() # clear the console output
    return (result, 'Failed to run test application')

  # Tests the clear button
  def testClear(self):
    test.object('Console').setText('Remove This')
    test.clear()
    result = test.object('Console').toPlainText() == ''
    return (result, 'Failed to clear text')
