#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import sys
import re
import vtk
import logging
import traceback
import inspect
from moosetools import parameters
from moosetools import mooseutils
from moosetools import base

class ChiggerObjectBase(base.MooseObject):
    """
    Base for all user-facing object in chigger.

    Inputs:
        **kwargs: key/value pairs that are used to update the options defined in the validParams
                  method
    """
   # __LOG_LEVEL__ = dict(critical=logging.CRITICAL, error=logging.ERROR, warning=logging.warning,
   #                      info=logging.INFO, debug=logging.DEBUG, notset=logging.NOTSET)

    @staticmethod
    def validParams():
        """
        Objects should define a static validParams method to add new key, value options.
        """
        from .. import utils # avoids cyclic dependencies
        params = utils.ChiggerInputParameters()
        params.add('name', vtype=str,
                doc="The object name (this name is displayed on the console help by pressing 'h'). "
                    "If a name is not supplied the class name is utilized.")


        levels = tuple(logging._nameToLevel.keys())
        params.add('log_level',
                   default='INFO',
                   vtype=str,
                   allow=levels,
                   mutable=False,
                   doc="Set the logging level, see python 'logging' package for details.")
        params.add(
            'log_status_error_level',
            default='ERROR',
            vtype=str,
            allow=levels,
            doc="Set the allowable logging level for the status method to return an error code.")

        return params

    def __init__(self, *args, **kwargs):
        base.MooseObject.__init__(self, *args, **kwargs)
        self.parameters().setErrorMode(parameters.InputParameters.ErrorMode.ERROR)

    def setParam(self, *args):
        """
        Set a specific option or sub-option

        Inputs:
            name(s) [str]: Name(s) for option or nested option
            value: The value to assign to the option or nested option

        Examples:
            setParam('year', 1980)
            setParam('date', 'year', 1980) # 'date' is an InputParameters object
            setParam('date_year', 1980)    # 'date' is an InputParameters object
        """
        #self.debug('setParam')
        self._parameters.setValue(*args)

    def setParams(self, *args, **kwargs):
        """
        A method for setting/updating an objects options.

        Usage:
           setParams(sub0, sub1, ..., key0=value0, key1=value1, ...)
           Updates all sub-options with the provided key value pairs

           setParams(key0=value0, key1=value1, ...)
           Updates the main options with the provided key,value pairs
        """
        self.debug('setParams')

        # Sub-options case
        if args:
            for sub in args:
                if not self._parameters.hasParameter(sub):
                    msg = "The supplied sub-option '{}' does not exist.".format(sub)
                    mooseutils.mooseError(msg)
                else:
                    self._parameters.getValue(sub).update(**kwargs)

        # Main options case
        else:
            self._parameters.update(**kwargs)

    def assignParam(self, *args):
        """
        Retrieve an option value and pass it into the given function
        """
        self.debug('assignParam')
        self._parameters.assign(*args)

    def printParam(self, key):
        """
        Prints the key/value pairing of an option as would be written in a script
        """
        print('{}={}'.format(key, repr(self.getParam(key))))

    def __setParamsFromCommandLine(self, argv):
        """
        Allow command-line modification of options upon during object construction.

        There are two syntax options:
            Type:Name:key=value
            Type:key=value
            The <name> is the value given to the 'name' option. If not provided then all objects of
            the type are changed.

        For example:
            Window:size=(500,400)
            Window:the_name_given:size=(500,400)
        """
        pattern = r'(?P<type>{}):(?:(?P<name>.*?):)?(?P<key>.*?)=(?P<value>.*)'.format(self.__class__.__name__)
        regex = re.compile(pattern)
        for arg in argv:
            match = re.search(regex, arg)
            if match and ((match.group('name') is None) or match.group('name') == self.name()):
                self.info('Setting Option from Command Line: {}'.format(match.group(0)))
                self.setParam(match.group('key'), eval(match.group('value')))

class ChiggerObject(ChiggerObjectBase):
    """
    Base class for objects that need options but are NOT in the VTK pipeline, for objects
    within the pipeline please use ChiggerAlgorithm.
    """

    def __init__(self, **kwargs):
        self.__modified_time = vtk.vtkTimeStamp()
        ChiggerObjectBase.__init__(self, **kwargs)
        self.__modified_time.Modified()

    def setParam(self, *args):
        """
        Set the supplied option, if anything changes mark the class as modified for VTK.

        See ChiggerObjectBase.setParam
        """
        ChiggerObjectBase.setParam(self, *args)
        if self._parameters.modified() > self.__modified_time.GetMTime():
            self.__modified_time.Modified()

    def setParams(self, *args, **kwargs):
        """
        Set the supplied options, if anything changes mark the class as modified for VTK.

        See ChiggerObjectBase.setParams
        """
        ChiggerObjectBase.setParams(self, *args, **kwargs)
        if self._parameters.modified() > self.__modified_time.GetMTime():
            self.__modified_time.Modified()
