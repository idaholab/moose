from sets import Set

class InputParameters:
  def __init__(self):
    self.valid = {}
    self.desc = {}

  def addRequiredParam(self, name, description):
    self.desc[name] = description

  def addParam(self, name, *args):
    if len(args) == 2:
      self.valid[name] = args[0]
    self.desc[name] = args[-1]

  def isValid(self, name):
    return name in self.valid

  def __contains__(self, item):
    return item in self.valid

  def __getitem__(self, key):
    return self.valid[key]

  def __setitem__(self, key, value):
    self.valid[key] = value

  def printParams(self):
    for k in self.desc:
      value = ''
      if k in self.valid:
        value = self.valid[k]

      print k.ljust(20), value
      print ' '.ljust(20), self.desc[k]