#!/usr/bin/python
import os, sys, time
from PySide import QtGui

import src.execute
from src.utils import TestObject

# Create the application
main = QtGui.QMainWindow()
test = src.execute.ExecuteWidget(main=main, testing=True, debug=False)

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
    test.object('Arguments').setText('')
    run = test.callback('Run')
    test.clear() # clear the console output
    run()
    test.process.waitForFinished()
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
    test.process.waitForFinished()
    result = test.object('Console').toPlainText() == 'Testing with flag'
    test.clear() # clear the console output
    return (result, 'Failed to run test application')

  # Tests the clear method via the callback
  def testClear(self):
    test.object('Console').setText('Remove This')
    callback = test.callback('ClearConsole')
    callback()
    result = test.object('Console').toPlainText() == ''
    return (result, 'Failed to clear text')

  # Tests the kill method via the callback
  def testKill(self):
    # Setup the executeable with the --kill flag
    test.object('Arguments').setText('--kill')
    test.object('Executable').setText(self.test_app)

    # Get the run/kill function callbacks
    run = test.callback('Run')
    kill = test.callback('KillConsole')

    # Clear the console run and kill the process
    test.clear() # clear the console output
    run()
    kill()

    # If the process was killed there should be nothing printed to the console
    test.process.waitForFinished()
    result = test.object('Console').toPlainText() == ''
    test.clear()
    return (result, 'Failed to kill application')
