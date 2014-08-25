#!/usr/bin/python

# Import Peacock modules
import utils

##
# A a class for creating self testing classes
#
# Any class that inherits from this interface has a test() method, this
# method automatically calls any methods with the '_test' prefix (e.g.,
# _testDataRead). The test method must meet the following criteria:
#     (a) It must be a non-static member variable (i.e., it takes self)
#     (b) It must not accept any additional arguments
#     (c) It must return a tuple containing the test result and the failure message,
#         (i.e., result, msg = _testThis())
class PeacockTestInterface(object):

  ## Constructor (empty; public)
  def __init__(self, **kwargs):
    self._tests = []

  ##
  # Register a function to execute as a test
  #
  #
  def registerTest(self, test):
    self._tests.append(test)

  ##
  # Performs the testing by calling all _test<YourNameHere> methods
  def test(self):

    for test in self._tests:
      # If it is not callable, i.e., a method, go to the next one
      if not hasattr(test, '__call__'):
        print "Some error message here"

      result, msg = test()
      self._showResult(test.__name__, result, msg)
    return

  ##
  # Show the pass/fail result in typical MOOSE run_tests style
  # @param name The name of the test, MyTest, for the method '_testMyTest'
  # @param result True/False result of the test
  # @param msg (optional) A message to show upon failure
  def _showResult(self, name, result, *args):

    # Build the status message: OK or FAIL

    if result:
       msg = utils.colorText('OK', 'GREEN')
       msg_length = 2
    else:
      # Add the failure message, if it exists
      msg_length = 4
      msg = ''
      if len(args) == 1 and isinstance(args[0], str) and len(args[0]) > 0 :
        msg = utils.colorText('(' + args[0] + ') ', 'RED')
        msg_length += len(args[0])+ 3

      # Print the failure
      msg += utils.colorText('FAIL', 'RED')

    # Produce the complete test message string
    name = utils.colorText(self.__class__.__name__, 'YELLOW') + '/' + name
    n = 110 - len(name) - msg_length
    print name + '.'*n + msg
