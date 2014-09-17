#!/usr/bin/python
import sys, os, inspect, time
from StringIO import StringIO

# Import Peacock modules
from src import utils

##
# A a class for performing testing
#
# Any class that inherits from this interface has a test() method that
# calls any function that is registered using 'registerTest' method.
# The function must output the test result (True | False) and a message
# that will show when the test fails
class PeacockTestInterface(object):

  ## Constructor (empty; public)
  def __init__(self, **kwargs):

    self._options = dict()
    self._options['verbose'] = kwargs.pop('verbose', False)
    self._options['quiet'] = kwargs.pop('quiet', False)


    self._tests = []

  ##
  # Register a function to execute as a test
  # @param test A function that is serving as a test
  # @param name (optional) The name of the test, the function name is used by default
  def registerTest(self, test, name = None):
    if name == None:
      name = test.__name__
    self._tests.append(dict(name=name, attr=test))

  ##
  # Performs the testing by calling all functions register via registerTest()
  def test(self):


    start_time = time.time()
    num_tests = len(self._tests)
    failed_tests = 0

    # This needs to be threaded
    for test in self._tests:
      name = test['name']
      attr = test['attr']

      backup = sys.stdout
      sys.stdout = StringIO()     # capture output

      (result, msg) = attr()

      if not result:
        failed_tests += 1

      out = sys.stdout.getvalue() # release output

      sys.stdout.close()  # close the stream
      sys.stdout = backup # restore original stdout

      self._showTestResult(name, result, msg, out)

    # Print the summary
    print '-'*110
    print 'Executed', num_tests, 'in', str(time.time() - start_time), 'seconds'
    print  utils.colorText(str(num_tests - failed_tests) + ' passed', 'GREEN') + ', ' \
           + utils.colorText(str(failed_tests) + ' failed', 'RED')


  ##
  # Show the pass/fail result in typical MOOSE run_tests style
  # @param name The name of the test
  # @param result True/False result of the test
  # @param msg A message to show upon failure
  # @param stdout The stdout string from the test execution, prints if failed
  def _showTestResult(self, name, result, msg, *args):

    # Build the status message: OK or FAIL
    if result:
      color = 'GREEN'
      msg = utils.colorText('OK', color)
      msg_length = 2
    else:
      color = 'RED'
      msg_length = len(msg) + 7
      msg = utils.colorText('(' + msg + ') FAIL', color)

    # Produce the complete test message string
    n = 110 - len(name) - msg_length
    print name + '.'*n + msg

    # Do not show any test output if --quiet is used
    if self._options['quiet']:
      return

    # Print the error message, if the test fails
    if not self._options['quiet'] and len(args) > 0 and (not result or (self._options['verbose'])):
      for line in args[0].splitlines():
        print utils.colorText(name + ': ', color) + line
