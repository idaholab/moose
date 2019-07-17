#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script finds a file in the herd trunk containing all the possible applications
# thay may be built with an "up" target.  If passed the value ROOT it will simply
# return the root directory

import os, sys, re, subprocess

def findDepApps(dep_names, use_current_only=False):
    dep_name = dep_names.split('~')[0]

    app_dirs = []
    moose_apps = ['framework', 'moose', 'test', 'unit', 'modules', 'examples']
    apps = []

    # First see if we are in a git repo
    p = subprocess.Popen('git rev-parse --show-cdup', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    p.wait()
    if p.returncode == 0:
        git_dir = p.communicate()[0]
        root_dir = os.path.abspath(os.path.join(os.getcwd(), git_dir)).rstrip()

        # Assume that any application we care about is always a peer
        dir_to_append = '.' if use_current_only else '..'
        app_dirs.append(os.path.abspath(os.path.join(root_dir, dir_to_append)))

    # Now see if we can find .build_apps in a parent directory from where we are at, usually "projects"
    restrict_file = '.build_apps'
    restrict_file_path = ''
    restrict_dir = ''

    next_dir = os.getcwd()
    for i in range(4):
        next_dir = os.path.join(next_dir, "..")
        if os.path.isfile(os.path.join(next_dir, restrict_file)):
            restrict_file_path = os.path.join(next_dir, restrict_file)
            break
    if restrict_file_path != '':
        restrict_dir = os.path.dirname(os.path.abspath(restrict_file_path))
        app_dirs.append(restrict_dir)

    # Make sure that we found at least one directory to search
    if len(app_dirs) == 0:
        return ''

    # unique paths to search
    unique_dirs = set()
    for dir in app_dirs:
        unique_dirs.add(os.path.abspath(dir))

    remove_dirs = set()
    # now strip common paths
    for dir1 in unique_dirs:
        for dir2 in unique_dirs:
            if dir1 == dir2:
                continue

            if dir1 in dir2:
                remove_dirs.add(dir2)
            elif dir2 in dir1:
                remove_dirs.add(dir1)
    # set difference
    unique_dirs = unique_dirs - remove_dirs

    if restrict_file_path != '':
        f = open(restrict_file_path)
        apps.extend(f.read().splitlines())
        f.close()

    # See which apps in this file are children or dependents of this app
    dep_apps = set()
    dep_dirs = set()

    # moose, elk and modules have special rules
    if dep_name == "moose":
        dep_app_re=re.compile(r"\bmoose\.mk\b")
    elif dep_name == "modules":
        dep_app_re=re.compile(r"\bmodules\.mk\b")
    elif dep_name == "elk":
        dep_app_re=re.compile(r"\belk(?:_module)?\.mk\b")
    else:
        dep_app_re=re.compile(r"^\s*APPLICATION_NAME\s*:=\s*"+dep_name,re.MULTILINE)

    ignores = ['.git', '.svn', '.libs', 'gold', 'src', 'include', 'contrib', 'tests', 'bak', 'tutorials']

    for dir in unique_dirs:
        startinglevel = dir.count(os.sep)
        for dirpath, dirnames, filenames in os.walk(dir, topdown=True):
            # Don't traverse too deep!
            if dirpath.count(os.sep) - startinglevel >= 2: # 2 levels outta be enough for anybody
                dirnames[:] = []

            # Don't traverse into ignored directories
            for ignore in ignores:
                if ignore in dirnames:
                    dirnames.remove(ignore)

            # Honor user ignored directories
            if os.path.isfile(os.path.join(dirpath, '.moose_ignore')):
                dirnames[:] = []
                continue

            # Don't traverse into submodules
            if os.path.isfile(os.path.join(dirpath, '.gitmodules')):
                f = open(os.path.join(dirpath, '.gitmodules'))
                content = f.read()
                f.close()
                sub_mods = re.findall(r'path = (\w+)', content)
                dirnames[:] = [x for x in dirnames if x not in sub_mods]

            potential_makefile = os.path.join(dirpath, 'Makefile')

            if os.path.isfile(potential_makefile):
                f = open(potential_makefile)
                lines = f.read()
                f.close()

                # We only want to build certain applications, look at the path to make a decision
                # If we are in trunk, we will honor .build_apps.  If we aren't, then we'll add it
                eligible_app = dirpath.split('/')[-1]

                if dep_app_re.search(lines) and ((len(apps) == 0 or eligible_app in apps) or ('/moose/' in dirpath and eligible_app in moose_apps)):
                    dep_apps.add(eligible_app)
                    dep_dirs.add(dirpath)

                    # Don't traverse once we've found a dependency
                    dirnames[:] = []

    # Now we need to filter out duplicate moose apps
    moose_dir = os.environ.get('MOOSE_DIR')
    return '\n'.join(dep_dirs)

if __name__ == '__main__':
    if len(sys.argv) == 2:
        dep_apps = findDepApps(sys.argv[1], False)
        print dep_apps
