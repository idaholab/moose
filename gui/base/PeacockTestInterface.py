#!/usr/bin/python

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
class PeacockTestInterface:

  ## Constructor (empty; public)
  def __init__(self, **kwargs):
    pass

  ##
  # Performs the testing by calling all _test<YourNameHere> methods
  def test(self):
    # Loop over all properties and methods for this class
    prefix = '_test'
    for item in dir(self):

      # If the method/property starts with _test, investigate for running
      if item.startswith(prefix):

        # Extract the test name and retrieve the attribute
        name = item.replace(prefix, '')
        attr = getattr(self, prefix+name)

        # If it is not callable, i.e., a method, go to the next one
        if not hasattr(attr, '__call__'):
          continue

        # Call the methods if it takes a single argument (self) otherwise produce
        # a failure with appropriate message
        args = attr.func_code.co_argcount
        if args == 1:
          result, msg = attr()
          self._showResult(name, result, msg)
        elif args == 0:
          self._showResult(name, False, prefix+name + ' cannot be static')
        else:
          self._showResult(name, False, prefix+name + ' cannot accept arguments')
    return

  ##
  # Show the pass/fail result in typical MOOSE run_tests style
  # @param name The name of the test, MyTest, for the method '_testMyTest'
  # @param result True/False result of the test
  # @param msg (optional) A message to show upon failure
  def _showResult(self, name, result, *args):

    # Build the status message: OK or FAIL
    if result:
      result = 'OK'
    else:
      result = 'FAIL'

      # Add the failure message, if it exists
      if len(args) == 1 and isinstance(args[0], str) and len(args[0]) > 0 :
        result += ' (' + args[0] + ')'

    # Produce the complete test message string
    name = self.__class__.__name__ + '/' + name
    n = 110 - len(name) - len(result)
    print name + '.'*n + result
