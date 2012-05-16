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
    exet = os.path.split(self.app_path)
    if exet[len(exet) - 1] != '':
      exet = exet[len(exet) - 1].split('/').pop()
    else:
      exet = exet[0].split('/').pop()
    EXTENSIONS = [ 'opt', 'dbg', 'pro' ]
    fname = None
    timestamp = time.time() + 99 #initialize to a big number (in the future)
    for ext in EXTENSIONS:
      exe = self.app_path + '/' + exet + '-' + ext
      print exe
      if os.path.isfile(exe):
        if os.path.getmtime(exe) < timestamp:
          fname = exe
          break
    if fname == None:
      print 'ERROR: You must build an ' + \
            'executable in ' + self.app_path + ' first.'
      sys.exit(1)
    data = commands.getoutput( fname + " --yaml" )
    data = data.split('**START YAML DATA**\n')[1]
    data = data.split('**END YAML DATA**')[0]
    if not os.path.exists(exet + '_yaml_dump'):
      data = yaml.load(data)
      pickle.dump(data, open(exet + '_yaml_dump', 'wb'))
    else:
      data = pickle.load(open(exet + '_yaml_dump', 'rb'))
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
