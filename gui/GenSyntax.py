#!/usr/bin/python
import sys, os, yaml, pickle, commands, time

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
  def __init__(self, app_path, recache):
    self.app_path = app_path
    self.recache = recache #Whether or not to force a yaml recache

  def GetSyntax(self):
    if not os.path.isfile(self.app_path):
      print 'ERROR: Executable ' + self.app_path + ' not found!'
      sys.exit(1)
    executable = os.path.basename(self.app_path)
    executable_path = os.path.dirname(self.app_path)
    yaml_dump_file_name = executable_path + '/yaml_dump_' + executable
    if self.recache or not os.path.exists(yaml_dump_file_name):
      data = commands.getoutput( self.app_path + " --yaml" )
      data = data.split('**START YAML DATA**\n')[1]
      data = data.split('**END YAML DATA**')[0]
      print "\nCaching syntax. Subsequent startup times will be greatly reduced."
      if not self.recache:
        print "In the future, you can use '-r' to force a syntax recache."
      
      data = yaml.load(data)
      pickle.dump(data, open(yaml_dump_file_name, 'wb'))
    else:
      data = pickle.load(open(yaml_dump_file_name, 'rb'))
#    data = self.massage_data(data)
#    for i in data:
#      printYaml(i, 0)
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
