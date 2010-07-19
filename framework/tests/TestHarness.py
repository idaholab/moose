import os, sys, re
from optparse import OptionParser
from socket import gethostname

from PerfLogParser import PerfLogParser
from ExcelWriter import ExcelWriter

import timeit, inspect, StringIO
class TestHarness:
  # static variable so the helper functions can access the executable name
  # using only the class name (not an instance)
  # this is cleaner than a global variable
  exec_name = ''

  def __init__(self, argv, app_name):
    # parse command line args
    self.options, self.leftovers, self.arg_string = self.getoptions(argv)

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

    # See if an Excel dump was requested
    if self.options.xlsFile:
      self.xls_writer = ExcelWriter(self.options.xlsFile)
    else:
      self.xls_writer = None

  def run_tests(self):
    out_string = ''
    results_table = ''
    test_counter = 0
    test_dir = os.getcwd()

    start = timeit.default_timer()
    for dirpath, dirnames, filenames in os.walk(test_dir):
      if (self.test_match.search(dirpath)):
        for file in filenames:
          # See if there were other arguments (test names) passed on the command line
          if (file[-2:] == 'py' and 
              self.test_match.search(file) and
              (len(self.leftovers) == 0 or len([item for item in self.leftovers if re.match(item, file)]) == 1)):
            module_name = file[:-3]

            result, result_string = self.inspectAndTest(dirpath, module_name)
            out_string += result_string
            if (result != 'skipped'):
              test_counter += 1
	    cnt = 70 - len(file + result) - 2
	    s = file + " " + '.'*cnt + " " + result
            print s
            results_table += s + "\n"
    end = timeit.default_timer()

    if self.xls_writer:
      self.xls_writer.close()
    return out_string, results_table, test_counter, str(round(end-start,3))

  # checks if this is icestorm, runs the tests, prints the results, and exits
  def run_tests_and_exit(self, check_icestorm=True):
    host_name = gethostname()
    if check_icestorm and (host_name == 'service0' or host_name == 'service1'):
      print 'Testing not supported on Icestorm head node'
      sys.exit(0)

    results, results_table, test_counter, time = self.run_tests()
    # Was it completely successful?
    print "\n" + '-'*70 + "\nRan " + str(test_counter) + " tests in " + time + "s\n\nOK"
    if results:
      print "\n" + results + results_table + "\n" + '-'*70 + "\nRan " + str(test_counter) + " tests in " + time + "s\n\nOK"
      sys.exit(1)

    sys.exit(0)

  def inspectAndTest(self, dirpath, module_name):
    saved_cwd = os.getcwd()  
    sys.path.append(os.path.abspath(dirpath))
    os.chdir(dirpath)

    # dynamically load the module
    module = __import__(module_name)

    result_string = ''
    # inspect the routine and look for test functions
    for routine, address in inspect.getmembers(module, inspect.isroutine):
      if self.test_match.search(routine):
                 
        test_result = 'ok'
        supported_args = set(inspect.getargspec(address)[0])
        try:
          # See if this test function supports benchmarking or parallel requests
          if ((self.arg_string.find('dofs') >= 0 and not 'dofs' in supported_args) or (self.arg_string.find('np') >=0 and not 'np' in supported_args)):
            test_result = 'skipped'
          else:
            test_start = timeit.default_timer()
               
            # Capture stdout to a buffer object for the local function call
            saved_stdout = sys.stdout
            capture = StringIO.StringIO()
            sys.stdout = capture

            eval('address(' + self.arg_string + ')')
                  
            sys.stdout = saved_stdout
            test_end = timeit.default_timer()
            if (self.arg_string == ''):
              test_result = 'OK'
            else:
              test_result = str(round(test_end-test_start,3)) + 's'
              if self.xls_writer:
                parser = PerfLogParser()
                parser.parse(capture.getvalue())
                parser.writeExcel(self.xls_writer, module_name)
            if (self.options.verbose == True):
              result_string += capture.getvalue()
                  
                        
        except AssertionError:
          sys.stdout = saved_stdout
          test_end = timeit.default_timer()

          result_string += capture.getvalue()
          test_result = 'FAILED'

    os.chdir(saved_cwd)
    sys.path.pop()

    return (test_result, result_string)
     

  def getoptions(self, argv): 
    # Callback function
    arg_vector = []
    def buildArgVector(option, opt_str, value, parser):
      arg_vector.append(option.dest + '=' +  str(value)) 

    parser = OptionParser()
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False, help="show more output [default=FALSE]")
    parser.add_option("-x", "--xls", action="store", dest="xlsFile", metavar="FILE", help="write excel format performance data to FILE")
    parser.add_option("-d", "--dofs", action="callback", callback=buildArgVector, type="int", dest="dofs", help="refine each example to meet the minimum requested DOFS")
    parser.add_option("-n", "--np", action="callback", callback=buildArgVector, type="int", dest="np", help="specify the number of MPI processes launched")
    parser.add_option("--opt", action="store_const", dest="method", const="opt", help="test the app_name-opt binary")
    parser.add_option("--dbg", action="store_const", dest="method", const="dbg", help="test the app_name-dbg binary")
    parser.add_option("--dev", action="store_const", dest="method", const="dev", help="test the app_name-dev binary")

    (options, args) = parser.parse_args(argv[1:])
    # Return the 'options' and leftover args returned from parse_args and
    # also the arg_string made from joining the arguments in the arg_vector
    return (options, args, ','.join(arg_vector))
