#!/usr/bin/env python

# This script finds a file in the herd trunk containing all the possible applications
# thay may be built with an "up" target.  If passed the value ROOT it will simply
# return the root directory

import os, sys, re, subprocess

def findDepApps(dep_names):
  dep_name = dep_names.split('~')[0]

  app_dirs = []
  apps = ['framework', 'moose', 'test', 'unit', 'modules']

  # First see if we are in a git repo
  p = subprocess.Popen('git rev-parse --show-cdup', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
  p.wait()
  if p.returncode == 0:
    git_dir = p.communicate()[0]
    app_dirs.append(os.path.abspath(os.path.join(os.getcwd(), git_dir)).rstrip())

  # Now see if we can find .build_apps which exists in the herd repo
  apps_file = '.build_apps'
  found_it = False
  apps_dir = os.getcwd()
  for i in range(4):
    apps_dir = os.path.join(apps_dir, "..")
    if os.path.exists(os.path.join(apps_dir, apps_file)):
      found_it = True
      break
  if found_it:
    app_dirs.append(os.path.abspath(apps_dir))

  # Finally see if HERD_TRUNK_DIR is defined
  herd_trunk = os.environ.get('HERD_TRUNK_DIR')
  if herd_trunk != None:
    app_dirs.append(herd_trunk)

  # Make sure that we found at least one directory to search
  if len(app_dirs) == 0:
    sys.exit(0)

#  print '\n'.join(app_dirs)

#  # Are we just looking for the ROOT?
#  if dep_name == 'ROOT':
#    return apps_dir

  # unique paths to search
  unique_dirs = set()
  for dir in app_dirs:
    unique_dirs.add(os.path.abspath(dir))

  for dir in unique_dirs:
    app_file = os.path.join(dir, apps_file)
    if os.path.isfile(app_file):
      # Read list of all apps
      f = open(app_file)
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

  ignores = ['.git', '.svn', '.libs', 'gold', 'src', 'include', 'contrib', 'tests']
  for dir in unique_dirs:
    for dirpath, dirnames, filenames in os.walk(dir):

      # Don't traverse into ignored directories
      for ignore in ignores:
        if ignore in dirnames:
          dirnames.remove(ignore)

      potential_makefile = os.path.join(dirpath, 'Makefile')

      if os.path.exists(potential_makefile):
        f = open(potential_makefile)
        lines = f.read()
        f.close()

        # We only want to build certain applications, look at the path to make a decision
        eligible_app = dirpath.split('/')[-1]
        if dep_app_re.search(lines) and eligible_app in apps and eligible_app not in dep_apps:
          dep_apps.add(eligible_app)
          dep_dirs.add(dirpath)

  return '\n'.join(dep_dirs)

if __name__ == '__main__':
  if len(sys.argv) == 2:
    dep_apps = findDepApps(sys.argv[1])
    print dep_apps
