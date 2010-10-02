from TestHarness import TestHarness

from pysqlite2 import dbapi2 as sqlite
import os, time

class TestTimer(TestHarness):
  def __init__(self, argv, app_name):
    TestHarness.__init__(self, argv, app_name)
    self.app_name = app_name

    if not self.options.revision:
      raise Exception, 'You must provide a revision # to store in the database'

    if not self.arg_string.find('dofs') >= 0:
      raise Exception, 'You must provide dofs to get timing information'

    self.db_file = self.options.dbFile
    if not self.db_file:
      home = os.environ['HOME']
      self.db_file = os.path.join(home, 'timingDB/timing.sqlite')
      print 'Warning: Assuming database location ' + self.db_file

  # After the run store the results in the database
  def postRun(self):
    TestHarness.postRun(self)

    con = sqlite.connect(self.db_file)
    cr = con.cursor()

    timestamp = int(time.time())
    load = os.getloadavg()[0]
    rev = int(self.options.revision)
    app = self.app_name
    dofs = self.parseDofs()

    # accumulate the test results
    data = []
    sum_time = 0
    num = 0
    parse_failed = False
    for test, result in self.results_table.items():
      if result == 'OK' or result == 'skipped' or result == 'FAILED' or result == 'FAIL':
        continue

      try:
        timing = float(result[:-1])
        sum_time += timing
        num += 1
        data.append( (app, test, rev, timestamp, timing, dofs) )
      except:
        parse_failed = True     #ignore with error message, and don't do average
        print 'ERROR: no timing info in string: ' + result

    # Don't compute the average if some failed
    if self.all_passed and not parse_failed and num > 0:
      average = sum_time / num
      data.append( (app, '_average_'+app, rev, timestamp, average, dofs) )

    # to prevent duplicates delete possible old values first
    cr.execute('delete from timing where app_name = ? and revision = ?', (app, rev))
    cr.executemany('insert into timing values (?,?,?,?,?,?)', data)
    con.commit()

  def addOptions(self, parser):
    TestHarness.addOptions(self, parser)

    parser.add_option("--store-timing", action="store_true", dest="time", default=False, help="Store timing in the database")
    parser.add_option("-r", "--revision", action="store", dest="revision", help="REQUIRED: the current revision")
    parser.add_option("--database", action="store", dest="dbFile", metavar="FILE", help="Location of database")

  # helper function to parser degrees of freedom from the arg_string
  def parseDofs(self):
    args = self.arg_string.split(',')
    for arg in args:
      lhs, rhs = arg.split('=')
      if lhs == 'dofs':
        return int(rhs)

    # we'll only get here if the user didn't provide dofs (or someone changed the args code)
    raise Exception, 'You must provide dofs to get timing information'
