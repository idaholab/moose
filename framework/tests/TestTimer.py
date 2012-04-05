from TestHarness import TestHarness
from options import *
import os, time, sys

CREATE_TABLE = """create table timing
(
  app_name text,
  test_name text,
  revision int,
  date int,
  seconds real,
  scale int,
  load real
);"""

class TestTimer(TestHarness):
  def __init__(self, argv, app_name, moose_dir):
    TestHarness.__init__(self, argv, app_name, moose_dir)
    try:
      from sqlite3 import dbapi2 as sqlite
    except:
      print 'Error: --store-timing requires the sqlite3 python module.'
      sys.exit(1)
    self.app_name = app_name
    self.db_file = self.options.dbFile
    if not self.db_file:
      home = os.environ['HOME']
      self.db_file = os.path.join(home, 'timingDB/timing.sqlite')
      if not os.path.exists(self.db_file):
        print 'Warning: creating new database at default location: ' + str(self.db_file)
        self.createDB(self.db_file)
      else:
        print 'Warning: Assuming database location ' + self.db_file

  def createDB(self, fname):
    from sqlite3 import dbapi2 as sqlite
    print 'Creating empty database at ' + fname
    con = sqlite.connect(fname)
    cr = con.cursor()
    cr.execute(CREATE_TABLE)
    con.commit()

  def preRun(self):
    from sqlite3 import dbapi2 as sqlite
    # Delete previous data if app_name and repo revision are found
    con = sqlite.connect(self.db_file)
    cr = con.cursor()
    cr.execute('delete from timing where app_name = ? and revision = ?', (self.app_name, self.options.revision))
    con.commit()

  # After the run store the results in the database
  def postRun(self, test, timing):
    from sqlite3 import dbapi2 as sqlite
    con = sqlite.connect(self.db_file)
    cr = con.cursor()

    timestamp = int(time.time())
    load = os.getloadavg()[0]

    # accumulate the test results
    data = []
    sum_time = 0
    num = 0
    parse_failed = False
    # Were only interested in storing scaled data
    if timing != None and test[SCALE_REFINE] != 0:
      sum_time += float(timing)
      num += 1
      data.append( (self.app_name, test[TEST_NAME], self.options.revision, timestamp, timing, test[SCALE_REFINE], load) )
    # Insert the data into the database
    cr.executemany('insert into timing values (?,?,?,?,?,?,?)', data)
    con.commit()
