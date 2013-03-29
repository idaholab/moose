class Factory:
  def __init__(self):
    self.objects = {}   # The registered Objects array

  def register(self, type, name):
    self.objects[name] = type

  def getValidParams(self, type):
    return self.objects[type].getValidParams()

  def create(self, type, params):
    return self.objects[type]('object', params)

