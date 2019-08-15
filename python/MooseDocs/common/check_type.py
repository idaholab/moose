#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tools for performing type checks that raise exceptions.
"""
import types
from .exceptions import MooseDocsException
def check_type(name, var, var_type):
    """
    Perform a type check that when if fails throws and exception.

    Inputs:
        name[str]: The name of the variable for message.
        var[...]: The varible to check.
        var_type[type or tuple(types...)]: The type(s) that the variable should be.

        NOTE: If a types.FunctionType is provided in var_type then the 'callable' method is
              used instead of isinstance.
    """
    if var_type is types.FunctionType:
        if not callable(var):
            msg = "The argument '{}' must be callable but {} was provided."
            raise MooseDocsException(msg, name, type(var))
    else:
        if not isinstance(var, var_type):
            msg = "The argument '{}' must be of type {} but {} was provided."
            raise MooseDocsException(msg, name, var_type, type(var))
