#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
from collections import OrderedDict
import enum
import logging
import copy

from .Parameter import Parameter

class InputParameters(object):
    """
    A warehouse for creating and storing options
    """

    class ErrorMode(enum.Enum):
        """Defines the error mode."""
        WARNING = 1  # logging.warning
        ERROR = 2    # logging.error
        EXCEPTION =3 # logging.critical and raises InputParametersException

    class InputParameterException(Exception):
        """Custom Exception used in for ErrorMode.EXCEPTION"""
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)

    LOG = logging.getLogger('InputParameters')

    def __init__(self, mode=ErrorMode.WARNING):
        self.__mode = mode
        self.__parameters = OrderedDict()

    def add(self, *args, **kwargs):
        """
        Add a new Parameter to the warehouse

        Inputs:
            see Parameter.py
        """
        if args[0] in self.__parameters:
            self.__errorHelper("Cannot add parameter, the parameter '{}' already exists.", args[0])
            return

        default = kwargs.get('default', None)
        if isinstance(default, InputParameters):
            kwargs['vtype'] = InputParameters

        self.__parameters[args[0]] = Parameter(*args, **kwargs)

    def __contains__(self, name):
        """
        Allows keys to be tested via "in" keyword.
        """
        return name in self.__parameters

    def __iadd__(self, params):
        """
        Append another Parameter objects options into this container.

        Inputs:
           options[InputParameters]: An instance of an InputParameters object to append.
        """
        for key in params.keys():
            self.__parameters[key] = params._InputParameters__parameters[key]
        return self

    def items(self):
        """
        Provides dict.items() functionality.
        """
        for name, param in self.__parameters.items():
            yield name, param.value

    def values(self):
        """
        Provides dict.values() functionality.
        """
        for param in self.__parameters.values():
            yield param.value

    def keys(self):
        """
        Provides dict.keys() functionality.
        """
        return self.__parameters.keys()

    def remove(self, name):
        """
        Remove an option from the warehouse.

        Inputs:
            name[str]: The name of the Parameter to remove
        """
        if name not in self.__parameters:
            self.__errorHelper("Cannot remove parameter, the parameter '{}' does not exist.", name)
        else:
            self.__parameters.pop(name)

    def isValid(self, name):
        """
        Test if the given option is valid (i.e., !None). (public)

        Inputs:
            name[str]: The name of the Parameter to retrieve
        """
        opt = self.__parameters.get(name, None)
        if opt is None:
            self.__errorHelper("Cannot determine if the parameters is valid, the parameter '{}' does not exist.", name)
            return None
        else:
            return opt.value is not None

    def setDefault(self, name, default):
        """
        Set the default value, this will onluy set the value if it is None
        """
        opt = self.__parameters.get(name, None)
        if opt is None:
            self.__errorHelper("Cannot set default, the parameter '{}' does not exist.", name)
        else:
            opt.default = default

    def getDefault(self, name):
        """
        Return the default value
        """
        opt = self.__parameters.get(name, None)
        if opt is None:
            self.__errorHelper("Cannot get default, the parameter '{}' does not exist.", name)
            return None
        return opt.default

    def isDefault(self, name):
        """
        Return True if the supplied option is set to the default value.

        Inputs:
            name[str]: The name of the Parameter to test.
        """
        opt = self.__parameters.get(name, None)
        if opt is None:
            self.__errorHelper("Cannot determine if the parameter is default, the parameter '{}' does not exist.", name)
            return None
        return opt.value == opt.default

    def set(self, name, *args, **kwargs):
        """
        Set the value of a parameter or update contents of a sub-parameters.

        If the associated parameter is an InputParameters object and the value is a dict, update
        the parameters via InputParameters.update

        Use:
           params.set('foo', 42)            # foo is an 'int'
           params.set('bar', year=1980)     # bar is an 'InputParameters' object
           params.set('bar', {'year':1980}) # bar is an 'InputParameters' object

        Inputs:
            name[str]: The name of the Parameter to modify
            value|kwargs: The value to set the parameter or key, value pairs if the parameter is
                          an InputParameters object
        """
        opt = self.__parameters.get(name, None)

        if opt is None:
            self.__errorHelper("Cannot set value, the parameter '{}' does not exist.", name)

        elif isinstance(opt.value, InputParameters):
            if len(args) > 0 and kwargs:
                self.__errorHelper("Key, value pairs are not allowed when setting the '{}' parameter with a supplied dict argument.", name)
            elif len(args) == 1 and isinstance(args[0], dict):
                opt.value.update(**args[0])
            elif len(args) == 1 and isinstance(args[0], InputParameters):
                opt.value = args[0]
            elif len(args) == 1:
                self.__errorHelper("The second argument for the '{}' parameter must be a dict() or InputParametrs object.", name)
            else:
                opt.value.update(**kwargs)
        else:
            if len(args) != 1:
                self.__errorHelper("A single second argument is required for the '{}' parameter.", name)
            elif kwargs:
                self.__errorHelper("Key, value pairs are not allowed when setting the '{}' parameter.", name)
            else:
                opt.value = args[0]

    def get(self, name):
        """
        Overload for accessing the parameter value by name with []

        Inputs:
            name[str]: The name of the Parameter to retrieve
        """
        opt = self.__parameters.get(name, None)
        if opt is None:
            self.__errorHelper("Cannot get value, the parameter '{}' does not exist.", name)
            return None
        else:
            return opt.value

    def hasParameter(self, name):
        """
        Test that the parameter exists.

        Inputs:
            name[str]: The name of the Parameter to check
        """
        return name in self.__parameters

    def update(self, *args, **kwargs):
        """"
        Update the options given key, value pairs

        Inputs:
            *args: InputParameters objects to use for updating this object.
            **kwargs: key, values pairs to used for updating this object.
        """
        # Unused options
        unused = set()

        # Update from InputParameters object
        for opt in args:
            if not isinstance(opt, InputParameters):
                self.__errorHelper("The supplied arguments must be InputParameters objects or key, value pairs.")
            else:
                for key in opt.keys():
                    if self.hasParameter(key):
                        self.set(key, opt.get(key))
                    else:
                        unused.add(key)

        # Update from kwargs
        for k, v in kwargs.items():
            if k in self.keys():
                self.set(k, v)
            else:
                unused.add(k)

        # Warning for unused
        if len(unused) > 0:
            msg = 'The following parameters do not exist: {}'
            self.__errorHelper(msg, ' '.join(unused))

    def __str__(self):
        """
        Allows print to work with this class.
        """
        return self.toString()

    def toString(self):
        """
        Create a string of all parameters using Parameter.toString
        """
        out = []
        for param in self.itervalues():
            out.append(param.toString())
        return '\n\n'.join(out)

    def __errorHelper(self, text, *args, **kwargs):
        """
        Produce warning, error, or exception based on operation mode.
        """
        msg = text.format(*args, **kwargs)
        if self.__mode == InputParameters.ErrorMode.WARNING:
            self.LOG.warning(msg)
        elif self.__mode == InputParameters.ErrorMode.ERROR:
            self.LOG.error(msg)
        else:
            self.LOG.critical(msg)
            raise InputParameters.InputParameterException(msg)
