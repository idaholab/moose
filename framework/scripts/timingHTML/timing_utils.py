#!/usr/bin/env python
import sys, os, shutil
from pysqlite2 import dbapi2 as sqlite

################################################################################
############### Generating html and json data from database ####################
################################################################################
class HTMLGen:
  def __init__(self, app_name, con):
    self.app_name = app_name
    self.con = con
    self.cr = self.con.cursor()
    self.ex = self.cr.execute

    # used to find moose resources, assume we are in trunk right now
    self.trunk_dir = '.' 
    # the generated html goes here
    self.base_dir = os.path.join(self.trunk_dir, 'html')
    shutil.rmtree(self.base_dir, ignore_errors=True)
    os.mkdir(self.base_dir)

  # generates the html and json data for this app
  def generateHTML(self):
    self.ex('select distinct test_name, dofs from timing where app_name = ?', (self.app_name,))
    tests = self.cr.fetchall()

    # generate the app.html file containing the checkboxes
    self.generateAppHTML(tests)

    # now generate a json data file for each test
    # each file has three parts, the timing listed by time, timing listed by
    # revision, and other info listed by revision
    base = os.path.join(self.base_dir, self.app_name)
    for test in tests:
      dofs = test[1]
      test = test[0]
      json = JSON_TEMPLATE.replace('$LABEL$', self.app_name + '.' + test)

      # TODO order by seconds for second select
      self.ex('select revision, date, seconds from timing where app_name = ? and test_name = ? order by revision',
                   (self.app_name,test))
      results = self.cr.fetchall()
      # fill out revision vs timing
      data = ['[' + str(r[0]) + ', ' + str(r[2]) + ']' for r in results]
      data = '[ ' + ', '.join(data) + ' ]'
      json = json.replace('$REV_DATA$', data)

      # fill out data vs timing
      data = ['[' + str(r[1]) + ', ' + str(r[2]) + ']' for r in results]
      data = '[ ' + ', '.join(data) + ' ]'
      json = json.replace('$TIME_DATA$', data)

      fname = os.path.join(base, test + '.json')
      f = open(fname, 'w')
      f.write(json)
      f.close()

  # generates the app.html file that contains the list of checkboxes
  def generateAppHTML(self, tests):
    tests = [CHECKBOX_TEMPLATE.replace('$TEST$', test[0]) for test in tests]
    html = '\n'.join(tests) + CHECKBOX_END

    base = os.path.join(self.base_dir, self.app_name)
    os.mkdir(base)
    f = open( os.path.join(base, self.app_name + '.html'), 'w' )
    f.write(html)
    f.close()

# templates to generate html out of
JSON_TEMPLATE = """{
  byrev:
  {
    label: "$LABEL$",
    data: $REV_DATA$
  },
  bytime:
  {
    label: "$LABEL$",
    data: $TIME_DATA$
  },
  info: "$INFO$"
}"""
CHECKBOX_TEMPLATE = '<div class="test"><input class="check" type="checkbox" id="$TEST$"></input><label for="$TEST$">$TEST$</label></div>'
CHECKBOX_END = '\n<br clear="all"/>'

################################################################################
####################### Database utility functions #############################
################################################################################
def createDB(fname):
  print 'Creating empty database at ' + fname
  con = sqlite.connect(fname)
  cr = con.cursor()
  cr.execute(CREATE_TABLE)
  con.commit()

def dumpDB(fname):
  print 'Dumping database at ' + fname
  con = sqlite.connect(fname)
  cr = con.cursor()
  ex = cr.execute
  ex('select * from timing')
  rows = cr.fetchall()
  for row in rows:
    print row


CREATE_TABLE = """create table timing
(
  app_name text,
  test_name text,
  revision int,
  date int,
  seconds real,
  dofs int
);"""

if __name__ == '__main__':
  home = os.environ['HOME']
  fname = os.path.join(home, 'timingDB/timing.sqlite')

  if '-c' in sys.argv or '--create' in sys.argv:
    createDB(fname)
  elif '-d' in sys.argv or '--dump' in sys.argv:
    dumpDB(fname)
  else: # '-h' in sys.argv or '--html' in sys.argv:
    con = sqlite.connect(fname)
    gen = HTMLGen('moose_test', con)
    gen.generateHTML()
