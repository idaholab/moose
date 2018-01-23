#!/usr/bin/env python

# This script checks and can optionally update MOOSE source files.
# You should always run this script without the "-u" option
# first to make sure there is a clean dry run of the files that should
# be updated

import os, string, re
from optparse import OptionParser

global_ignores = ['contrib', '.svn', '.git', 'libmesh']

unified_header = \
"""//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html"""

global_options = {}

def fixupHeader():
    for dirpath, dirnames, filenames in os.walk(os.getcwd() + "/../../"):

        # Don't traverse into ignored directories
        for ignore in global_ignores:
            if ignore in dirnames:
                dirnames.remove(ignore)

        #print dirpath
        #print dirnames
        for file in filenames:
            suffix = os.path.splitext(file)
            if suffix[-1] == '.C' or suffix[-1] == '.h':
                checkAndUpdate(os.path.abspath(dirpath + '/' + file))


def checkAndUpdate(filename):
    f = open(filename)
    text = f.read()
    f.close()

    # Use the copyright header for framework files, use the lgpl header
    # for everything else
    header = unified_header

    # Check (exact match only)
    if (string.find(text, header) == -1):
        # print the first 10 lines or so of the file
        if global_options.update == False: # Report only
            print filename + ' does not contain an up to date header'
            if global_options.verbose == True:
                print '>'*40, '\n', '\n'.join((text.split('\n', 10))[:10]), '\n'*5
        else:
            # Make sure any previous C-style header version is removed
            text = re.sub(r'^/\*+/$.*^/\*+/$', '', text, flags=re.S | re.M)

            # Make sure that any previous C++-style header (with extra character)
            # is also removed.
            text = re.sub(r'(?:^//\*.*\n)*', '', text, flags=re.M)

            # Now cleanup extra blank lines
            text = re.sub(r'^\s*\n$', '', text, flags=re.M)

            # Update
            f = open(filename + '~tmp', 'w')
            f.write(header + '\n')

            f.write(text)
            f.close()
            os.rename(filename + '~tmp', filename)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option("-u", "--update", action="store_true", dest="update", default=False)
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False)
    (global_options, args) = parser.parse_args()
    fixupHeader()
