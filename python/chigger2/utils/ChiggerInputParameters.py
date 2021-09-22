#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from collections import OrderedDict
import copy
from moosetools import mooseutils
from moosetools import parameters
from .ChiggerParameter import ChiggerParameter
from .AutoColor import Color

class ChiggerInputParameters(parameters.InputParameters):
    """
    A warehouse for creating and storing options
    """
    __PARAM_TYPE__ = ChiggerParameter

    def modified(self):
        """
        Returns the maximum modified time for the Option/Parameter objects
        """
        return max(opt.modified for opt in self._InputParameters__parameters.values())

    def assign(self, *args):
        """
        Helper for assign values to a function, if they option is valid
        """
        param = self._getParameter(*args[:-1])
        if (param is not None) and (param.value is not None):
            if isinstance(param.value, Color):
                args[-1](param.value.rgb())
            else:
                args[-1](param.value)

    def toDict(self, *keys):
        """
        Return a dict() from the supplied keys
        """
        return {k:self.getValue(k) for k,v in self.items() if v is not None}

    def toScript(self, **kwargs):
        """
        Takes an Params object and returns a string for building python scripts.

        Inputs:
            kwargs: Key, value pairs provided will replace values in options with the string given
                    in the value, but will not set the actual value. This is generally for
                    code generation tools.
        """
        output = []
        sub_output = dict()
        for key in self.keys():
            opt = self.getValue(key)

            if isinstance(opt, Params):
                items, _ = opt.toScript()
                if items:
                    sub_output[key] = items

            elif not self.isDefault(key):
                if key in kwargs:
                    r = kwargs[key]
                else:
                    r = repr(opt)
                output.append('{}={}'.format(key, r))

        return output, sub_output

    def getNonDefaultParams(self, **kwargs):
        output = []
        sub_output = dict()
        for key in self.keys():
            opt = self.getValue(key)

            if isinstance(opt, ChiggerInputParameters):
                items, _ = opt.getNonDefaultParams()
                if items:
                    sub_output[key] = items

            elif not self._parameters.isDefault(key):
                if key in kwargs:
                    r = kwargs[key]
                else:
                    r = repr(opt)
                output.append(key)

        return output, sub_output
