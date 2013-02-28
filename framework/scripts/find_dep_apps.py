#!/usr/bin/env python

# This script finds a file in the herd trunk containing all the possible applications
# thay may be built with an "up" target.  If passed the value ROOT it will simply
# return the root directory

import os, sys, re

def findDepApps(dep_names):
  apps_file = '.build_apps'

  dep_name = dep_names.split('~')[0]

  # Locate the .build_apps file
  found_it = False
  apps_dir = os.getcwd() + "/"
  for i in range(4):
    apps_dir += "../"
    if os.path.exists(apps_dir + apps_file):
      found_it = True
      break
  if not found_it:
    print "ERROR: Unable to find '.build_apps' file in your parent directory tree"
    sys.exit(1)

  # Are we just looking for the ROOT?
  if dep_name == 'ROOT':
    return apps_dir

  # Read list of all apps
  f = open(apps_dir + apps_file)
  apps = f.read().splitlines()
  f.close()

  # See which apps in this file are children or dependents of this app
  dep_apps = []
  appNameDirRE=re.compile("^\s*"+dep_name.upper()+"_DIR",re.MULTILINE)
  for app in apps:
    if os.path.exists(apps_dir + app + '/Makefile'):
      f = open(apps_dir + app + '/Makefile')
      lines = f.read()
      f.close()

      # See if this app is listed in the Makefile
      if app.upper() not in [x.upper() for x in dep_names.split('~')] and appNameDirRE.search(lines):
        dep_apps.append(app)

  return ' '.join(dep_apps)

# Entry point
if len(sys.argv) == 2:
  dep_apps = findDepApps(sys.argv[1])
  print dep_apps
