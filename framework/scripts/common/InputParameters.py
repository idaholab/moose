class InputParameters:
  def __init__(self):
    self.valid = {}
    self.desc = {}
    self.substitute = {}
    self.required = set()

  def addRequiredParam(self, name, *args):
    self.required.add(name)
    self.addParam(name, *args)

  def addParam(self, name, *args):
    if len(args) == 2:
      self.valid[name] = args[0]
    self.desc[name] = args[-1]

  def addStringSubParam(self, name, substitution, *args):
    self.substitute[name] = substitution
    self.addParam(name, *args)

  def isValid(self, name):
    if name in self.valid and self.valid[name] != None and self.valid[name] != []:
      return True
    else:
      return False

  def __contains__(self, item):
    return item in self.desc

  def __getitem__(self, key):
    return self.valid[key]

  def __setitem__(self, key, value):
    self.valid[key] = value

  def type(self, key):
    if key in self.valid:
      return type(self.valid[key])
    else:
      return None

  def keys(self):
    return set([k for k in self.desc])

  def required_keys(self):
    return self.required

  def valid_keys(self):
    return self.valid

  def substitute_keys(self):
    return self.substitute

  def isRequired(self, key):
    return key in self.required

  def getDescription(self, key):
    return self.desc[key]

  def printParams(self):
    for k in self.desc:
      value = ''
      if k in self.valid:
        value = self.valid[k]

      print k.ljust(20), value
      print ' '.ljust(20), self.desc[k]
