#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .InputParameters import InputParameters

# This is the base class from which all objects should inherit
class MooseObject(object):

    @staticmethod
    def validParams():
        params = InputParameters()

        return params

    ##
    # MooseObject can be built via two methods:
    #   MooseObject(name, params)
    #   MooseObject(params)
    def __init__(self, *args, **kwargs):

        if len(args) == 1 and isinstance(args[0], InputParameters):
            self._name = None
            self._pars = args[0]

        elif len(args) == 2 and isinstance(args[0], str) and isinstance(args[1], InputParameters):
            self._name = args[0]
            self._pars = args[1]

    ##
    # Return the name of the object
    #
    # If the object was built with name, params arguements it will return the
    # supplied name in the constructor. If the name was not given in the constructor
    # and the "name" parameter is defined it will return the value of this parameter.
    # Otherwise None will be returned.
    def name(self):
        if (self._name is None) and self.isParamValid('name'):
            self._name = self.getParam('name')
        return self._name

    def parameters(self):
        return self._pars

    def getParam(self, name):
        return self._pars[name]

    def isParamValid(self, name):
        return self._pars.isValid(name)
