import os, sys, re
from optparse import OptionParser
from socket import gethostname

from PerfLogParser import PerfLogParser
from ExcelWriter import ExcelWriter

import timeit, inspect, StringIO

class ExodiffException(Exception):
  pass

class TestHarness:
  # static variable so the helper functions can access the executable name
  # using only the class name (not an instance) from tools.py
  # this is cleaner than a global variable
  exec_name = ''

  def __init__(self, argv, app_name):
    # parse command line args
    self.options, self.leftovers, self.arg_string = self.getOptions(argv)

    # Determine the application name based on the libMesh naming scheme
    # Assume the application is in the current working directory
    # Order of importance, 1. command line args 2. environment 3. default is opt
    method = ''
    if self.options.method:
      method = self.options.method
    elif os.environ.has_key('METHOD'):
      method = os.environ['METHOD']
    else:
      method = 'opt'
    executable = os.getcwd() + '/' + app_name + '-' + method

    TestHarness.exec_name = executable

    # Emulate the standard Nose RegEx for consistency
    self.test_match = re.compile(r"(?:^|\b|[_-])[Tt]est")

    self.all_passed = True
    self.num_tests = 0
    self.num_passed = 0

    # map of tests to results
    self.results_table = {}

    # holds the string table dumped to the screen for easy printing
    self.text_results_table = ''

    # See if an Excel dump was requested
    if self.options.xlsFile:
      self.xls_writer = ExcelWriter(self.options.xlsFile)
    else:
      self.xls_writer = None

  # Call this function to run all tests and exit success or fail
  def runTestsAndExit(self, check_icestorm=True):
    # Check if this is icestorm before we run the tests
    host_name = gethostname()
    if check_icestorm and (host_name == 'service0' or host_name == 'service1'):
      print 'Testing not supported on Icestorm head node'
      sys.exit(0)

    # Call overridable functions to run and process the tests
    self.preRun()
    self.runAll()
    self.postRun()

    msg = self.getFailMessage()
    if msg:
      print BOLD + RED + msg + RESET
      sys.exit(1)
    sys.exit(0)

  # override this function if you need to do stuff before the tests are run
  def preRun(self):
    self.all_passed = True
    self.results_table = {}
    self.text_results_table = ''
    self.num_tests = 0

  # finds all the test py scripts and passes them to inspectAndTest
  def runAll(self):
    start = timeit.default_timer()

    for dirpath, dirnames, filenames in os.walk(os.getcwd()):
      if (self.test_match.search(dirpath)):
        for file in filenames:
          # See if there were other arguments (test names) passed on the command line
          if (file[-2:] == 'py' and 
              self.test_match.search(file) and
              (len(self.leftovers) == 0 or len([item for item in self.leftovers if re.match(item, file)]) == 1)):

            module_name = file[:-3]
            self.inspectAndTest(dirpath, module_name)

    end = timeit.default_timer()
    self.testing_time = end - start


  # takes a module name (.py file) and runs all the tests in it
  def inspectAndTest(self, dirpath, module_name):
    saved_cwd = os.getcwd()  
    sys.path.append(os.path.abspath(dirpath))
    os.chdir(dirpath)

    # dynamically load the module
    module = __import__(module_name)

    # inspect the routine and look for test functions
    for routine, address in inspect.getmembers(module, inspect.isroutine):
      if self.test_match.search(routine):
                 
        supported_args = set(inspect.getargspec(address)[0])
        testname = module_name + '.' + routine
        # See if this test function supports benchmarking or parallel requests
        if ((self.arg_string.find('dofs') >= 0 and not 'dofs' in supported_args) or 
            (self.arg_string.find('np') >=0 and not 'np' in supported_args) or 
            (self.arg_string.find('n_threads') >=0 and not 'n_threads' in supported_args)):
          self.runTest(testname, address, True)
        else:
          self.runTest(testname, address)
                        
    os.chdir(saved_cwd)
    sys.path.pop()

  # run a single test
  def runTest(self, test, address, override=False):
    result = ''
    try:
      try:
        s = test + " ..."
        self.text_results_table += '\n' + s
        sys.stdout.write(s)
        sys.stdout.flush()
        test_start = timeit.default_timer()
           
        # Capture stdout to a buffer object for the local function call
        saved_stdout = sys.stdout
        capture = StringIO.StringIO()
        sys.stdout = capture

        args = ''
        if override:
          args = ''
        else:
          args = self.arg_string

        eval('address(' + args + ')')

        test_end = timeit.default_timer()

        # confusing: if arg_string is populated it means we changed dofs/np so
        # we want to see the testing time, not just pass/fail
        if (args == ''):
          result = 'OK'
        else:
          result = str(round(test_end-test_start,3)) + 's'

      except AssertionError:
        self.all_passed = False
        result = 'FAILED'
      except ExodiffException:
        # A regular exception means that the program ran but produced different output
        self.all_passed = False
        result = 'FAILED (DIFF)'
    finally:
      sys.stdout = saved_stdout
      test_end = timeit.default_timer()
      output = capture.getvalue()
      
      # We can skip established tests with the "pass" keyword in our
      # python driver files in which case no output is produced.
      # In this case we want to notify the user that the test was actually
      # skipped not OK
      if output == '':
        result = 'skipped'
      else:
        self.num_tests += 1

      self.processOutput(test, result, capture.getvalue())


  # override in sub classes to process the output of each test
  def processOutput(self, test, result, output):
    if self.xls_writer:
      parser = PerfLogParser()
      parser.parse(output)
      parser.writeExcel(self.xls_writer, test)

    # print the result of this test in table form
    self.results_table[test] = result
    if result == 'OK':
      self.num_passed += 1
    cnt = 65 - len(test + result)
    s = '.'*cnt + ' ' + result
    self.text_results_table += s
    print self.colorify(s)

    # print the output if verbose option is on or the test failed
    if self.options.verbose or result.find('FAILED') != -1:
      print '*'*100 + '\n' + '*'*41 + '  FAILED OUTPUT  ' + '*'*42 + '\n' + '*'*100 + '\n' + output
      #print s

  # return '' if all tests passed, or a message if some failed. Override if you
  # want more fail criteria
  def getFailMessage(self):
    if self.all_passed:
      return ''
    return 'FAILED '

  # override this function if you need to do stuff after the tests are run
  def postRun(self):
    if self.xls_writer:
      self.xls_writer.close()

    # if there was output before this the full table will be obscured, so
    # print it in it's entirety here at the bottom
    if self.options.verbose or not self.all_passed:
      print '\n\nFull Result Table:\n' + '-'*70
      print self.colorify(self.text_results_table)

    print '-'*70
    print 'Ran ' + str(self.num_tests) + ' tests in ' + str(round(self.testing_time)) + 's (' + str(self.num_passed) + '/' + str(self.num_tests) + ' passed)\n'

  # deprecated because it violates the coding standard's naming scheme
  def run_tests_and_exit(self, check_icestorm=True):
    self.runTestsAndExit(check_icestorm)

  def getOptions(self, argv): 
    # Callback function
    arg_vector = []
    def buildArgVector(option, opt_str, value, parser):
      arg_vector.append(option.dest + '=' +  str(value)) 

    parser = OptionParser()
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="show more output [default=FALSE]")
    parser.add_option("-x", "--xls", action="store", dest="xlsFile", metavar="FILE", help="write excel format performance data to FILE")
    parser.add_option("-d", "--dofs", action="callback", callback=buildArgVector, type="int", dest="dofs", help="refine each example to meet the minimum requested DOFS")
    parser.add_option("-n", "--np", action="callback", callback=buildArgVector, type="int", dest="np", help="specify the number of MPI processes launched")
    parser.add_option("--helios-only-after", action="store_true", dest="heliosOnly", help="The options after this option are only applied if hostname == helios")
    parser.add_option("--opt", action="store_const", dest="method", const="opt", help="test the app_name-opt binary")
    parser.add_option("--dbg", action="store_const", dest="method", const="dbg", help="test the app_name-dbg binary")
    parser.add_option("--dev", action="store_const", dest="method", const="dev", help="test the app_name-dev binary")
    parser.add_option("-c", "--colored", action="store_true", dest="colored", default=False, help="Show colored output")
    parser.add_option("--n_threads", type="int", dest="n_threads", action="callback", callback=buildArgVector, help="specify the number of threads to be used")
    self.addOptions(parser)

    (options, args) = parser.parse_args(argv[1:])
    # Return the 'options' and leftover args returned from parse_args and
    # also the arg_string made from joining the arguments in the arg_vector
    return (options, args, ','.join(arg_vector))

  # override this method to add custom options to the parser
  def addOptions(self, parser):
    pass

  def colorify(self, str):
    if self.options.colored:
      str = str.replace('OK', BOLD+GREEN+'OK'+RESET)
      str = str.replace('skipped', BOLD+'skipped'+RESET)
      str = str.replace('FAILED (DIFF)', BOLD+YELLOW+'FAILED (DIFF)'+RESET)
      str = str.replace('FAILED', BOLD+RED+'FAILED'+RESET)

    return str


# ANSI color codes for colored terminal output
RESET  = '\033[0m'
BOLD   = '\033[1m'
RED    = '\033[31m'
GREEN  = '\033[32m'
YELLOW = '\033[33m'
