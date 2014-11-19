import sys, os, importlib, inspect, pkgutil, time
from StringIO import StringIO
from PySide import QtGui

from src.base import *
from src.utils import TestObject, colorText, PeacockTestObject
import tests

##
# A class for running all available test objects that are stored in the tests directory,
# test objects must inherit from src.utils.TestObject.
class PeacockTester(object):
  def __init__(self, **kwargs):

    # Parse tester options
    self._options = dict()
    self._options['verbose'] = kwargs.pop('verbose', False)
    self._options['quiet'] = kwargs.pop('quiet', False)

    # Initialize storage for the test objects
    self._test_objects = []

    # Create a QApplication object, only one of these is allowed at a time, so this
    # object is passed to the PeaocockTestObjects via the 'app' keyword value pair
    app  = QtGui.QApplication(sys.argv)

    # Loop through all modules in 'tests' and create instances of all PeacockTestObjects
    for importer, modname, ispkg in pkgutil.iter_modules(tests.__path__, tests.__name__ + '.'):
      module = __import__(modname, fromlist="dummy")
      for name, obj in inspect.getmembers(module):
        if inspect.isclass(obj):
          instance = obj(app=app)
          if isinstance(instance, PeacockTestObject):
            self._test_objects.append(instance)

  ##
  # Performs the testing by calling all functions
  def execute(self):

    start_time = time.time()
    num_tests = 0
    failed = 0

    # This needs to be threaded
    for obj in self._test_objects:
      for test in obj.tests:
        num_tests += 1
        name = test['name']
        attr = test['attr']

        # Capture the standard output
        backup = sys.stdout
        sys.stdout = StringIO()     # capture output

        # Perform the test
        (result, msg) = attr()
        if not result:
          failed += 1

        # Return the standard output
        out = sys.stdout.getvalue() # release output
        sys.stdout.close()  # close the stream
        sys.stdout = backup # restore original stdout

        # Dislpay the resulst
        self._showTestResult(name, result, msg, out)

      # Delete the PeacockTestObject that just completed running its tests
      del obj

    # Print the summary
    print '-'*110
    print 'Executed', str(num_tests), 'tests in', str(time.time() - start_time), 'seconds'
    print  colorText(str(num_tests - failed) + ' passed', 'GREEN') + ', ' \
           + colorText(str(failed) + ' failed', 'RED') + '\n'

  ##
  # Show the pass/fail result in typical MOOSE run_tests style
  # @param name The name of the test
  # @param result True/False result of the test
  # @param msg A message to show upon failure
  # @param stdout The stbdout string from the test execution, prints if failed
  def _showTestResult(self, name, result, msg, *args):

    # Build the status message: OK or FAIL
    if result:
      color = 'GREEN'
      msg = colorText('OK', color)
      msg_length = 2
    else:
      color = 'RED'
      msg_length = len(msg) + 7
      msg = colorText('(' + msg + ') FAIL', color)

    # Produce the complete test message string
    n = 110 - len(name) - msg_length
    print name + '.'*n + msg

    # Do not show any test output if --quiet is used
    if self._options['quiet']:
      return

    # Print the error message, if the test fails
    if not self._options['quiet'] and len(args) > 0 and (not result or (self._options['verbose'])):
      for line in args[0].splitlines():
        print colorText(name + ': ', color) + line
