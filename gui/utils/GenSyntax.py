#!/usr/bin/python
import sys, os, yaml, pickle, commands, time
from PyQt4 import QtCore, QtGui

def printYaml(data, level):
  indent_string = ''
  for i in xrange(0,level):
    indent_string += ' '
  print indent_string + str(data['name'])
  
  indent_string += ' '
  
  for item,value in data.items():
    if item != 'name' and item != 'parameters' and item != 'subblocks':
      print indent_string + item + ": " + str(value)
      
  if 'parameters' in data and data['parameters']:
    for param in data['parameters']:
      print indent_string + param['name']
      for name, value in param.items():
        print indent_string + ' ' + str(name) + ': ' + str(value)

  if data['subblocks']:
    for d in data['subblocks']:
      printYaml(d, level+1)    

class GenSyntax():
  def __init__(self, qt_app, app_path, use_cached_syntax):
    self.qt_app = qt_app
    self.app_path = app_path
    self.use_cached_syntax = use_cached_syntax
    self.saved_raw_data = None
    self.saved_data = None    
    
  def GetSyntax(self, recache):
    if not self.use_cached_syntax and not os.path.isfile(self.app_path):
      print 'ERROR: Executable ' + self.app_path + ' not found!'
      sys.exit(1)
    self.executable = os.path.basename(self.app_path)
    self.executable_path = os.path.dirname(self.app_path)
    yaml_dump_file_name = self.executable_path + '/yaml_dump_' + self.executable
    raw_yaml_dump_file_name = self.executable_path + '/yaml_dump_' + self.executable + '_raw'
    
    raw_data = self.getRawDump()

    if not self.saved_raw_data:
      if os.path.isfile(raw_yaml_dump_file_name):
        self.saved_raw_data = pickle.load(open(raw_yaml_dump_file_name, 'rb'))
      else:
        recache = True

    if not recache:
      if self.saved_raw_data != raw_data: # If the yaml has changed - force a recache
        recache = True
      elif self.saved_data: #If we have currently loaded data - just return it!
        return self.saved_data

    if recache or not os.path.exists(yaml_dump_file_name) or not os.path.exists(raw_yaml_dump_file_name):
      progress = QtGui.QProgressDialog("Recaching Syntax...", "Abort", 0, 10, None)
      progress.setWindowModality(QtCore.Qt.WindowModal)
      progress.show()
      progress.raise_()

      for i in xrange(0,7):
        progress.setValue(i)
        self.qt_app.processEvents()
        self.qt_app.flush()

      pickle.dump(raw_data, open(raw_yaml_dump_file_name, 'wb'))
      self.saved_raw_data = raw_data
      
      data = yaml.load(raw_data)
      pickle.dump(data, open(yaml_dump_file_name, 'wb'))

      progress.setValue(8)
      progress.setValue(9)
      progress.setValue(10)
    else:
      data = pickle.load(open(yaml_dump_file_name, 'rb'))

    self.saved_data = data
    return data

  def getRawDump(self):

    if not self.use_cached_syntax:
      data = commands.getoutput( self.app_path + " --yaml" )
      data = data.split('**START YAML DATA**\n')[1]
      data = data.split('**END YAML DATA**')[0]
    else:
      data = pickle.load(open(self.executable_path + '/yaml_dump_' + self.executable + '_raw', 'rb'))

    return data

  def massage_data(self, data):
    for block in data:
      name =  block['name']
      if name == 'Executioner' or name == 'InitialCondition':
        curr_type = str(block['type'])
        if curr_type == 'None':
          curr_type = 'ALL'
        block['name'] = name + '/' + curr_type
    return data
