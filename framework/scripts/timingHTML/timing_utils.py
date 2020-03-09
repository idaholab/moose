#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os, shutil
from socket import gethostname
from datetime import datetime

# if it doesn't import this is probably icestorm and we won't use it anyway
try:
    from sqlite3 import dbapi2 as sqlite
except:
    pass

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
        if not os.path.exists(self.base_dir):
            os.mkdir(self.base_dir)

    # generates the html and json data for this app
    def generateHTML(self):
        self.ex('select distinct test_name from timing where app_name = ?', (self.app_name,))
        tests = self.cr.fetchall()
        tests = [test[0] for test in tests]

        # generate the app.html file containing the checkboxes
        self.generateAppHTML(tests)

        # now generate a json data file for each test
        # each file has three parts, the timing listed by time, timing listed by
        # revision, and other info listed by revision
        base = os.path.join(self.base_dir, self.app_name)
        for test in tests:
            json = JSON_TEMPLATE.replace('$LABEL$', self.app_name + '.' + test)

            # fill out revision vs timing
            self.ex('select revision, seconds, date, scale, load from timing where app_name = ? and test_name = ? order by date',
                         (self.app_name,test))
            results = self.cr.fetchall()
            data = ['["' + str(r[0]) + '", ' + str(r[1]) + ']' for r in results]
            data = '[ ' + ', '.join(data) + ' ]'
            json = json.replace('$REV_DATA$', data)

            data = [ ( self.app_name, test, str(r[0]), str(r[1]), str(datetime.fromtimestamp(r[2])), str(r[3]), str(r[4]) ) for r in results ]
            data = [ '["' + '","'.join(d) + '"]' for d in data ]
            data = '[ ' + ', '.join(data) + ' ]'
            json = json.replace( '$INFO$', data )

            # fill out data vs timing
            # use another select statement because revisions and real dates may not exactly align
            self.ex('select date, seconds from timing where app_name = ? and test_name = ? order by date',
                         (self.app_name,test))
            results = self.cr.fetchall()
            data = ['[' + str(r[0]*1000) + ', ' + str(r[1]) + ']' for r in results]
            data = '[ ' + ', '.join(data) + ' ]'
            json = json.replace('$TIME_DATA$', data)

            fname = os.path.join(base, test + '.json')
            f = open(fname, 'w')
            f.write(json)
            f.close()

    # generates the app.html file that contains the list of checkboxes
    def generateAppHTML(self, tests):
        tests = [CHECKBOX_TEMPLATE.replace('$TEST$', test) for test in tests]
        html = '\n'.join(tests) + CHECKBOX_END

        base = os.path.join(self.base_dir, self.app_name)
        if not os.path.exists(base):
            os.mkdir(base)
        f = open( os.path.join(base, self.app_name + '.html'), 'w' )
        f.write(html)
        f.close()

# templates to generate html out of
JSON_TEMPLATE = """{
  "byrev":
  {
    "label": "$LABEL$",
    "data": $REV_DATA$
  },
  "bytime":
  {
    "label": "$LABEL$",
    "data": $TIME_DATA$
  },
  "info": $INFO$
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
  revision text,
  date int,
  seconds real,
  scale int,
  load real
);"""

HELP_STRING = """Usage:
-h   print this help message
-c   create database with table timing in ~/timingDB/timing.sqlite
     the database must either not exist or not have a timing table
-d   dump the contents of table timing

[list of applications]  using the data in the database, generate
     json data for every application in the list. Assume db at
     ~/timingDB/timing.sqlite
"""


if __name__ == '__main__':
    home = os.environ['HOME']
    fname = os.path.join(home, 'timingDB/timing.sqlite')
    argv = sys.argv[1:]

    if '-h' in argv:
        argv.remove('-h')
        print HELP_STRING
        sys.exit(0)

    if '-c' in argv:
        createDB(fname)
        argv.remove('-c')

    if '-d' in argv:
        dumpDB(fname)
        argv.remove('-d')

    if len(argv) > 0:
        con = sqlite.connect(fname)
        for app in argv:
            print "generating json data for " + app + "."
            gen = HTMLGen(app, con)
            gen.generateHTML()
