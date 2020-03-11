#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# A python script to remove outdated dependency files and associated objects. It operates from any
# directory within the herd. It cleans out the moose directory and any herd animials included in the
# Makefile.
#
# The script requires one input parameter, the root directory of MOOSE (e.g., ~/projects/trunk)

# Load the necessary packages
import os, sys, re

##  A function for locating dependency herd animials
# Reads "Makefile" and searches for *.mk includes to build a list of directories to search
# for dependencies
#
# @param root_dir The MOOSE root directory (e.g., ~/projects/trunk/)
#
# @return A list of directories to clean
#
def findDepDirs(root_dir):

    # Test the moose_dir is valid
    if not os.path.isdir(root_dir):
        print "ERROR: Supplied MOOSE root directory (" + root_dir + ") does not exist."
        sys.exit(1)

    # Open the Make file
    file = open(os.getcwd() + "/Makefile")

    # Storage for dependency directories, always include the moose directory
    if os.environ.get('MOOSE_DEV') == "true":
        dep_dirs = [os.path.join(root_dir, "devel", "moose")]
    else:
        dep_dirs = [os.path.join(root_dir, "moose")]

    # Loop through the file and look for include statements for .mk files
    for line in file:
        x = re.search('include .*\/(\w+)\.mk' , line)

        # Store all *.mk includes other than the moose.mk and build.mk
        if x and x.group(1) != "build" and x.group(1) != "moose":
            dep_dirs.append(os.path.join(root_dir, x.group(1)))

    # Close file, return the list of directories to clean
    file.close()
    return dep_dirs

## Cleans outdated dependencies for supplied directory
# Recursively searches the directory for *.d files, then searches the *.d file for headers (*.h).
# If the header does not exist the *.d and associated object file is removed.
#
# @param cur_dir Full path to the directory that needs to be cleaned
#
def cleanDepDirs(cur_dir):

    # Walk through, recursively, the sub-directories and build a list of directories to search
    dep_files = []
    for path, dirs, files in os.walk(cur_dir):

        # Define the top-level directory
        top = re.search('.*\/(.*)', path)
        top = top.group(1)

        # If the top-level is a dot directory or if it is the "doc" directory, do nothing,
        # else search for *.d files and append the list
        if not top.startswith(".") and not path.endswith("doc"):
            for f in files:
                if f.endswith(".d"):
                    dep_files.append(os.path.join(path, f))

    # Loop through all dep files
    for cur_dep_file in dep_files:

        # Open the *.d file, extract the lines, and close the file. The file should
        # be closed before continuing since it may get deleted below
        file = open(cur_dep_file)
        dep_file_lines = file.read().splitlines()
        file.close()

        # Loop through each line in the file, if it is a header, check that it exists
        # If it does not exist delete the *.d file and the coresponding object file
        for line in dep_file_lines:

            # Search line for header file name, ignore leading and trailing whitespace
            hdr = re.search('\s*(.*\.h)', line)

            # If the dep. file is a header and does not exist the dep. file is outdated
            if hdr and not os.path.isfile(hdr.group(1)):

                # Print a message that if the outdated dependency and remove the *.d  file
                print "    " + cur_dep_file + " is out of date, it is being removed"
                os.remove(cur_dep_file)

                # "Removing only the dependency file may be insufficient, and may in fact lead to
                # an incorrect build -- we also need to remove the object file associated with the
                # dependency file we removed...we can hopefully get the name of the object file by
                # stripping off the .d from the dependency file's filename." -JP
                x = re.search('(.*)\.d', cur_dep_file)
                if x and os.path.isfile(x.group(1)):

                    # Print a message for the outdated object file and remove it
                    print "    " + x.group(1) + " is out of date, it is being removed"
                    os.remove(x.group(1))

                # Stop the looping over the lines of this *.d file
                break

# ENTRY POINT
if len(sys.argv) == 2:

    # Print a message
    print "====== Cleaning outdated dependencies ======"

    # Locate the directories to clean
    dep_dirs = findDepDirs(sys.argv[1])

    # Clean each of the directories
    for dir in dep_dirs:
        print "  Cleaning: " + dir
        cleanDepDirs(dir)

else:

    # Report an error
    print "ERROR: You must supply the root MOOSE directory (e.g., ~/project/trunk)"
    sys.exit(1)
