#!/usr/bin/python
import os, sys
from PySide import QtGui

from src.execute import ExecuteWidget
from src.utils import TestObject

#if ('app' not in globals()):# or (not app.isinstance(QtGui.QApplication)):
#  app  = QtGui.QApplication(sys.argv)

class CSVIOTest(TestObject):
  def __init__(self, **kwargs):
    TestObject.__init__(self, **kwargs)

    self.test_file = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'input.csv')

    main = QtGui.QMainWindow()
    test = ExecuteWidget(main=main)

  # Test reading
  def testRun(self):

#  _test.info()
#  gamecock = PeacockApp(executable='test_app')
 # run = gamecock.tabs.callback('Run')
 # run()
    return (False, 'message')
#  return (result, fail_msg)
