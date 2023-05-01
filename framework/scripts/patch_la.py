#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script finds the current repository revision base on the log file
# It currently understands both local git-svn and svn repositories

import os, sys, re, shutil

def rewrite_dep_paths(filename, dep_lib_path):
    f = open(filename)
    buffer = f.read()
    f.close()

    depend_lib_pattern = re.compile(r"dependency_libs='\s*(.*)'")
    lines = buffer.split('\n')

    f = open(filename + '~tmp', 'w')
    for line in lines:
        m = depend_lib_pattern.match(line)
        if m is not None:
            line_to_process = m.group(1)
            libs = line_to_process.split()
            installed_libs = [os.path.join(dep_lib_path, os.path.basename(lib)) for lib in libs]
            line = "dependency_libs='" + ' '.join(installed_libs) + "'"

        f.write(line + '\n')
    f.close()

    shutil.copystat(filename, filename + '~tmp')
    os.rename(filename + '~tmp', filename)

if len(sys.argv) == 3:
    file_to_process = sys.argv[1]
    dep_lib_path = sys.argv[2]
    rewrite_dep_paths(file_to_process, dep_lib_path)
