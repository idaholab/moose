from TestHarness import TestHarness


class LeakDetector(TestHarness):
  def __init__(self, argv, app_name):
    TestHarness.__init__(self, argv, app_name)
    self.leakers = []

    # map of leaking tests to (leaked obj, num leaked) tuple if dbg executable
    self.objects = {}

  # add tests that leak memory to a list
  def processOutput(self, test, result, output):
    TestHarness.processOutput(self, test, result, output)

    index = output.find('Memory leak detected!')
    if index >= 0:
      self.leakers.append(test)
      
      # If it's a debug executable, find and parse the memory leaked objects
      if TestHarness.exec_name[-3:] == 'dbg':
        self.objects[test] = []
        try:
          end_table = output.rfind(' -------', 0, index)
          start_table = output.rfind(' ------', 0, end_table)
          start_table = output.find('\n', start_table ) + 1
          table = output[start_table:end_table].strip().split('\n')
          for i in range(len(table)/3):
            object = table[3*i].split(' ')[1]
            creations = int( table[3*i+1].split(' ')[-1] )
            destructions = int( table[3*i+2].split(' ')[-1] )
            leftover = creations - destructions
            if leftover != 0:
              self.objects[test].append( (object, str(leftover)) )
        except:
          self.objects[test].append( ('Error','Exception parsing object') )
          raise

  # print out error messages after the testharness output
  def postRun(self):
    TestHarness.postRun(self)

    if len(self.leakers) == 0:
      print 'No memory leaks detected!'
    else:
      print 'Memory leaks detected in ' + str(len(self.leakers)) + ' tests:'

      # if debug executable, print the memory leaking objects
      if TestHarness.exec_name[-3:] == 'dbg':
        for test, leaks in self.objects.items():
          print test
          for object, leftover in leaks:
            print ' - ' + object + ': ' + leftover
      else:
        print "(Run run_tests with --dbg to report what object's are being leaked)"
        print '\n'.join(self.leakers)

  def getFailMessage(self):
    msg = TestHarness.getFailMessage(self)
    if len(self.leakers) > 0 and self.options.die_on_leak:
      return msg + 'MEMORY LEAK '
    return msg

  def addOptions(self, parser):
    TestHarness.addOptions(self, parser)

    parser.add_option("--memcheck", action="store_true", dest="memcheck", default=False, help="Display tests that leak memory")
    parser.add_option("--dieonleak", action="store_true", dest="die_on_leak", default=False, help="Exit with an error when a test in the suite leaks memory. Depends on the --memcheck option.")
