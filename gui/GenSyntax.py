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
  def __init__(self, app_path):
    self.app_path = app_path

  def GetSyntax(self):
    if not os.path.isfile(self.app_path):
      print 'ERROR: Executable ' + self.app_path + ' not found!'
      sys.exit(1)
    executable = os.path.basename(self.app_path)
    data = commands.getoutput( self.app_path + " --yaml" )
    data = data.split('**START YAML DATA**\n')[1]
    data = data.split('**END YAML DATA**')[0]
    if not os.path.exists(executable + '_yaml_dump'):
      data = yaml.load(data)
      pickle.dump(data, open(executable + '_yaml_dump', 'wb'))
    else:
      data = pickle.load(open(executable + '_yaml_dump', 'rb'))
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
