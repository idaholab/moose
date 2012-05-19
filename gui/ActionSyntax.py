#!/usr/bin/python
import sys, os, commands, time, re

class ActionSyntax():
  def __init__(self, app_path):
    self.app_path = app_path
    self.paths = []
    self.hard_paths = []

    if not os.path.isfile(self.app_path):
      print 'ERROR: Executable ' + self.app_path + ' not found!'
      sys.exit(1)
      
    executable = os.path.basename(self.app_path)

    data = commands.getoutput( self.app_path + " --syntax" )

    # The 1 is so we skip the first line
    self.paths = list(set(data.split('\n')[1:]))
    self.paths.sort()

    for path in self.paths:
      if path[len(path)-1] != '*':
        self.hard_paths.append(path)

  """ Whether or not this is a hard path """
  def isPath(self, inpath):
    path = inpath
    path = path.lstrip('/')
    for hard_path in self.hard_paths:
      modified = hard_path.replace('*','[^/]*')
      modified += '$'

      p = re.compile(modified)
    
      if p.match(path):
        return True
    return False

  """ Get back the Action path
      If this is not a hard path it will return None
      If this path is a hard path and does not need wildcard matching it will return the same path
      In the event of wildcard matching it will return a path with stars in it """
  def getPath(self, inpath):
    path = inpath
    path = path.lstrip('/')
    print path
    for hard_path in self.hard_paths:
      modified = hard_path.replace('*','[^/]*')
      modified += '$'

      p = re.compile(modified)
    
      if p.match(path):
        return hard_path
    return None

    

  """ Whether or not this path has a star after it """
  def hasStar(self, the_path):
    for path in self.paths:
      if path.find(the_path+'/*') != -1:
        return True
    return False

