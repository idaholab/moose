#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os, time, shutil
import subprocess # probably only available on unix-like OSes
from socket import gethostname

EXTENSIONS = [ 'opt', 'dbg', 'pro', 'oprof' ]

# Returns an absolute path to a folder containing the web page representing
# a tree of the blocks expected in the input file for a moose program.
#
# Parameters:
# app_name: the project name. Used for the title of the page and to locate the
#           executable to get data from. ex: bison or rat
#
# app_path: base directory to search for executable
#
# argv: the arguments passed to the script
#
# moose_path: the moose base directory.
#
# autocopy: if True, remove app_path/syntax/ then copy the new documentation there
def generateHTML( app_name, app_path, argv, moose_path = '../moose', autocopy = True ):
    # if we're running this from the build system we only want to run it once instead
    # of on all four targets. So bitten passes the --helios-only option, then this
    # script does nothing if it is not run on helios
    if len(argv) == 2 and argv[1] == '--helios-only' and gethostname() != 'helios':
        print('Syntax NOT generated because this is not helios')
        sys.exit(0)
    else:
        print('Generating syntax html')

    # Try to import the required modules and die gracefully if they don't load
    test_import_modules()

    # look for the most recently modified executable
    fname = None
    timestamp = time.time() + 99 #initialize to a big number (in the future)
    for ext in EXTENSIONS:
        exe = app_path + '/' + app_name + '-' + ext
        if os.path.isfile(exe):
            if os.path.getmtime(exe) < timestamp:
                fname = exe

    if fname == None:
        print('ERROR: You must build a ' + \
              app_name + ' executable in ' + app_path + ' first.')
        sys.exit(1)

    data = subprocess.getoutput( fname + " --yaml" )

    # test if YAML DATA is in the output. This is so we can give a clear error
    # message if the program doesn't run to completion (instead of an IndexError)
    if data.find( 'YAML DATA' ) < 0:
        print('File Data' + '-'*50 + '\n' + data)
        print('End File Data' + '-'*46)
        print('ERROR: The yaml data is not being printed by: ' + fname)
        sys.exit(1)

    # ignore the first part of the file, up to START YAML DATA
    data = data.split('**START YAML DATA**\n')[1]
    # ignore the last part of the file, after END YAML DATA
    data = data.split('**END YAML DATA**')[0]

    # ignore root node, start displaying all it's child nodes
    data = yaml.load( data )
    #data = data[0]['subblocks']

    # perform some changes to the data, like renaming executioners to their type
    data = massage_data( data )

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
        shutil.copytree( template_path + 'static', docBase + 'static', ignore=ignore_svns )

    # finally, write our generated html over index.html
    index = open( docBase + 'index.html', 'w' )
    index.write( html )
    index.close()

    # if autocopy, copy the documentation to the root application directory for them
    if autocopy:
        shutil.rmtree( app_path + '/syntax', ignore_errors = True )
        shutil.copytree( docBase, app_path + '/syntax' )

    return docBase

# modify the data tree
def massage_data( data ):
    for block in data:
        name =  block['name']
        if name == 'Executioner' or name == 'InitialCondition':
            curr_type = str(block['type'])
            if curr_type == 'None':
                curr_type = 'ALL'
            block['name'] += ' (' + curr_type + ')'
    return data

# used by copytree, don't copy any .svn files out to the server!
def ignore_svns( adir, filenames ):
    return [ filename for filename in filenames if filename == ".svn" ]

# tests to see if we can load yaml and Mako Templates. If not print an error
# message telling the user what to install
def test_import_modules():
    yaml_loaded = False
    mako_loaded = False
    try:
        global yaml
        import yaml
        yaml_loaded = True
    except ImportError:
        pass
    try:
        global Template
        from mako.template import Template
        mako_loaded = True
    except ImportError:
        pass

    msg = ''
    if not yaml_loaded:
        msg += 'ImportError: No module named yaml\nPlease easy_install PyYaml\n'
    if not mako_loaded:
        msg += 'ImportError: No module named mako.template\nPlease easy_install Mako\n'
    if not msg == '':
        print(msg)
        exit(1)
