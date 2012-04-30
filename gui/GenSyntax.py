#!/usr/bin/python
import sys, os, yaml, pickle, commands, time

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
    if fname == None:
      print 'ERROR: You must build an ' + \
            'executable in ' + self.app_path + ' first.'
      sys.exit(1)
    data = commands.getoutput( fname + " --yaml" )
    data = data.split('**START YAML DATA**\n')[1]
    data = data.split('**END YAML DATA**')[0]
    if not os.path.exists('yaml_dump'):
      data = yaml.load(data)
      pickle.dump(data, open('yaml_dump', 'wb'))
    else:
      data = pickle.load(open('yaml_dump', 'rb'))
    data = self.massage_data(data)
    return data

  def massage_data(self, data):
    for block in data:
      name =  block['name']
      if name == 'Executioner' or name == 'InitialCondition':
        curr_type = str(block['type'])
        if curr_type == 'None':
          curr_type = 'ALL'
        block['name'] += ' (' + curr_type + ')'
    return data
