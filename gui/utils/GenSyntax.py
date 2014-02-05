#!/usr/bin/python
import sys, os, yaml, pickle, commands, time
from PyQt4 import QtCore, QtGui

##
# A helper function for printYaml (private)
# Prints a Yaml data dictionary to the screen
# @param data The dictionary to print
# @param level The indentation level to utilize
# @see printYaml
def _printYamlDict(data, level=0):

  # Indent two spaces for each level
  indent = '  '

  # Iterate through the dictionary items
  for key,value in data.items():

    # Print the name as a header
    if key == 'name':
      print indent*level + str(value)

    # The subblocks contain additional dictionaries; loop
    # through each one and print at an increated indentation
    elif key == 'subblocks':
      if value != None:
        for v in value:
          _printYamlDict(v, level+1)

    # The parameters contain additional dictionaries; loop 
    # through the parameters and place the output under a parameter section
    elif key == 'parameters':
      print indent*(level+1) + 'parameters:'
      if value != None:
        for v in value:
          _printYamlDict(v, level+2)

    # The default case, print the key value pairings
    else:
      print (indent*(level+1) + str(key) + " = " + str(value)).rstrip('\n')

##
# A function for printing the YAML information to the screen (public)
# @param data The YAML dump data (returned by GenSyntax::GetSynatx)
# @param name Limits the output based on the supplied string, if the 
#             supplied name is anywhere in the 'name' parameter of the 
#             top level YAML data the corresponding dictionary is printed (optional)
def printYaml(data, name = None):
 
  # Print all output
  if name == None:
    for d in data:
      _printYamlDict(d)

  # Only print data that contains the given name string
  else:
    for d in data:
      if name in d['name']:
        _printYamlDict(d)
    
  
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
