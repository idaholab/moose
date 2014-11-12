import sys, os, inspect, time
from StringIO import StringIO

# Import Peacock modules
from colorText import colorText

##
# A a class for performing testing
#
# Any class that inherits from this interface has a test() method that
# calls any function that is registered using 'registerTest' method.
# The function must output the test result (True | False) and a message
# that will show when the test fails
class TestObject(object):

  ## Constructor (public)
  def __init__(self, **kwargs):

    # Parse tester options
    self._options = dict()
    self._options['verbose'] = kwargs.pop('verbose', False)
    self._options['quiet'] = kwargs.pop('quiet', False)

    # Build a list of tests to perform
    self._tests = []
    for func in dir(self):
      if func.startswith('test'):
        self._tests.append({'name':self.__class__.__name__+'/'+func, 'attr':getattr(self, func)})

  ##
  # Performs the testing by calling all functions
  def execute(self):

    start_time = time.time()
    self.num_tests = len(self._tests)
    self.failed = 0

    # This needs to be threaded
    for test in self._tests:
      name = test['name']
      attr = test['attr']

      backup = sys.stdout
      sys.stdout = StringIO()     # capture output

      (result, msg) = attr()
#      try:
#        (result, msg) = attr()
#      except:
#        result = False
#        msg = 'RUN ERROR'

      if not result:
        self.failed += 1

      out = sys.stdout.getvalue() # release output
      sys.stdout.close()  # close the stream
      sys.stdout = backup # restore original stdout

      self._showTestResult(name, result, msg, out)

    self.time = time.time() - start_time
    self.passed = self.num_tests - self.failed

  def summary(self):

    # Print the summary
    print '-'*110
    print 'Executed', str(self.num_tests), 'in', str(self.time), 'seconds'
    print  colorText(str(self.passed) + ' passed', 'GREEN') + ', ' \
           + colorText(str(self.failed) + ' failed', 'RED')


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
