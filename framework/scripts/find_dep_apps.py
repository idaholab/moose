#!/usr/bin/env python

# This script finds a file in the herd trunk containing all the possible applications
# thay may be built with an "up" target.  If passed the value ROOT it will simply
# return the root directory

import os, sys, re, subprocess

def findDepApps(dep_names):
  dep_name = dep_names.split('~')[0]

  app_dirs = []
  moose_apps = ['framework', 'moose', 'test', 'unit', 'modules', 'examples']
  apps = moose_apps[:]

  # First see if we are in a git repo
  p = subprocess.Popen('git rev-parse --show-cdup', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
  p.wait()
  if p.returncode == 0:
    git_dir = p.communicate()[0]
    root_dir = os.path.abspath(os.path.join(os.getcwd(), git_dir)).rstrip()
    app_dirs.append(os.path.abspath(os.path.join(root_dir, '..')))

  # Now see if we can find .build_apps which exists in the herd repo
  apps_file = '.build_apps'
  apps_file_path = ''
  apps_dir = os.getcwd()
  for i in range(4):
    apps_dir = os.path.join(apps_dir, "..")
    if os.path.isfile(os.path.join(apps_dir, apps_file)):
      apps_file_path = os.path.join(apps_dir, apps_file)
      break
  if apps_file_path != '':
    app_dirs.append(os.path.abspath(os.path.join(apps_dir, '..')))

  # Finally see if HERD_TRUNK_DIR is defined
  herd_trunk = os.environ.get('HERD_TRUNK_DIR')
  if herd_trunk != None:
    app_dirs.append(herd_trunk)
    if os.path.isfile(os.path.join(herd_trunk, apps_file)):
      apps_file_path = os.path.join(herd_trunk, apps_file)


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

  if apps_file_path != '':
    f = open(apps_file_path)
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

  ignores = ['.git', '.svn', '.libs', 'gold', 'src', 'include', 'contrib', 'tests', 'bak']
  for dir in unique_dirs:
    startinglevel = dir.count(os.sep)
    for dirpath, dirnames, filenames in os.walk(dir, topdown=True):
      # Don't traverse too deep!
      if dirpath.count(os.sep) - startinglevel >= 3: # 2 levels outta be enough for anybody
        dirnames[:] = []

      # Don't traverse into ignored directories
      for ignore in ignores:
        if ignore in dirnames:
          dirnames.remove(ignore)

      # Honor user ignored directories
      if os.path.isfile(os.path.join(dirpath, '.moose_ignore')):
        dirnames[:] = []
        continue

      potential_makefile = os.path.join(dirpath, 'Makefile')

      if os.path.isfile(potential_makefile):
        f = open(potential_makefile)
        lines = f.read()
        f.close()

        # We only want to build certain applications, look at the path to make a decision
        # If we are in trunk, we will honor .build_apps.  If we aren't, then we'll add it
        eligible_app = dirpath.split('/')[-1]
        if dep_app_re.search(lines) and (eligible_app in apps or '/trunk/' not in dirpath):
          dep_apps.add(eligible_app)
          dep_dirs.add(dirpath)

          # Don't traverse once we've found a dependency
          dirnames[:] = []


  # Now we need to filter out duplicate moose apps
  moose_dir = os.environ.get('MOOSE_DIR')
  return '\n'.join(dep_dirs)

if __name__ == '__main__':
  if len(sys.argv) == 2:
    dep_apps = findDepApps(sys.argv[1])
    print dep_apps
