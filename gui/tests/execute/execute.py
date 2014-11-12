#!/usr/bin/python
import os, sys
from PySide import QtGui

from src.execute import ExecuteWidget

#if ('app' not in globals()):# or (not app.isinstance(QtGui.QApplication)):
#  app  = QtGui.QApplication(sys.argv)
print 'app' in globals()
print 'app' in locals()
main = QtGui.QMainWindow()

test = ExecuteWidget(main=main)

# Test reading
def testRun():

#  _test.info()
#  gamecock = PeacockApp(executable='test_app')
 # run = gamecock.tabs.callback('Run')
 # run()
  return (False, 'message')
#  return (result, fail_msg)
