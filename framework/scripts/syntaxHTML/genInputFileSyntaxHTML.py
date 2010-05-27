#!/usr/bin/env python

import sys, os, time, shutil
import commands # probably only available on unix-like OSes
import yaml

# easy_install Mako
from mako.template import Template

EXTENSIONS = [ 'opt', 'dbg', 'dev' ]

# Returns an absolute path to a folder containing the web page representing
# a tree of the blocks expected in the input file for a moose program.
#
# Parameters:
# app_name: the project name. Used for the title of the page and to locate the
#           executable to get data from. ex: bison or rat
#
# app_path: base directory to search for executable
#
# moose_path: the moose base directory.
#
# autocopy: if True, remove app_path/syntax/ then copy the new documentation there
def generateHTML( app_name, app_path, moose_path = '../moose', autocopy = True ):
  fname = None
  timestamp = time.time() + 99 #initialize to a big number (in the future)

  # look for the most recently modified executable
  for ext in EXTENSIONS:
    exe = app_path + '/' + app_name + '-' + ext
    if os.path.isfile(exe):
      if os.path.getmtime(exe) < timestamp:
        fname = exe

  if fname == None:
    print 'ERROR: You must build a ' + \
          app_name + ' executable in ' + app_path + ' first.'
    sys.exit(1)

  #print fname
  data = commands.getoutput( fname + " --yaml" )

  #ignore the first part of the file, up to START YAML DATA
  data = data.split('**START YAML DATA**\n')[1]
  #ignore the last part of the file, after END YAML DATA
  data = data.split('**END YAML DATA**')[0]
  
  #ignore root node, start displaying all it's child nodes
  data = yaml.load( data )
  data = data[0]['subblocks']

  gentime = time.strftime( '%H:%M %B %d, %Y' )

  context = { 'data' : data,
              'project' : app_name.upper(),
              'time' : gentime }

  # render the Mako template
  template_path = moose_path + '/scripts/syntaxHTML/'
  temp = Template( filename = template_path + 'template.html' )
  html = temp.render( **context )

  # if a previous user removed or moved the doc directory, recreate it
  docBase = template_path + 'doc'
  if not os.path.exists( docBase ):
    os.mkdir( docBase )
  docBase += '/'

  # assume if the static dir still exists (ie a previous user didn't move it
  # away) then it is filled with all the js and css we need
  if not os.path.exists( docBase + 'static' ):
    shutil.copytree( template_path + 'static', docBase + 'static' )

  # finally, write our generated html over index.html
  index = open( docBase + 'index.html', 'w' )
  index.write( html )
  index.close()

  # if autocopy, copy the documentation to the root application directory for them
  if autocopy:
    shutil.rmtree( app_path + '/syntax', ignore_errors = True )
    shutil.copytree( docBase, app_path + '/syntax' )

  return docBase
