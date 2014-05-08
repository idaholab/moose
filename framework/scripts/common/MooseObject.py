from InputParameters import InputParameters

# This is the base class from which all objects should inherit
class MooseObject(object):
  # Static Method
  def validParams():
    params = InputParameters()

    return params
  validParams = staticmethod(validParams)

  def __init__(self, name, params):
    self._name = name
    self._pars = params

  def name(self):
    return self._name

  def parameters(self):
    return self._pars

  def getParam(self, name):
    return self._pars[name]
