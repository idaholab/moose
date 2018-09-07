#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mooseutils
from .. import utils

class ChiggerObject(object):
    """
    Base for all user-facing object in chigger.

    The primary purpose is to provide a method for getting key, value
    options and consistent update methods.
    """

    @staticmethod
    def validOptions():
        """
        Objects should define a static validOptions method to add new key, value options. (public)
        """
        opt = utils.Options()
        opt.add('name', vtype=str,
                doc="The object name (this name is displayed on the console help by pressing 'h')."
                    "If a name is not supplied the class name is utilized.")
        return opt

    def __init__(self, **kwargs):
        super(ChiggerObject, self).__init__()
        self._options = getattr(self.__class__, 'validOptions')()
        self._options.update(**kwargs)

    def __str__(self):
        return '{}:\n{}'.format(mooseutils.colorText(self.__class__.__name__, "GREY"),
                                self._options.string())

    def title(self):
        if self.isOptionValid('name'):
            title = '{} ({})'.format(self.getOption('name'), self.__class__.__name__)
        else:
            title = self.__class__.__name__
        return title

    def options(self):
        """
        Return the utils.Options object for this class. (public)
        """
        return self._options

    def update(self, **kwargs):
        """
        Update method should contain calls to underlying vtk objects. (public)

        Inputs:
            initialize[bool]: When True the initialize() method will be called, but only if needed.
        """
        mooseutils.mooseDebug("{}.update()".format(self.__class__.__name__))
        self._options.update(**kwargs)

    def isOptionValid(self, name):
        """
        Test if the given option is valid (i.e., not None). (public)
        """
        return self._options.isOptionValid(name)

    def getOption(self, name):
        """
        Return the value of an option. (public)

        Inputs:d
            name[str]: The name of the option to retrieve
        """
        if name not in self._options:
            msg = "The {} object does not contain the '{}' option.".format(self.title(), name)
            mooseutils.mooseWarning(msg)
            return None
        return self._options.get(name)

    def applyOption(self, name):
        """
        Returns the option and sets the apply
        """
        return self._options.applyOption(name)

    def setOption(self, name, value):
        """
        Set single option. (public)

        Inputs:
            name[str]: The name of the option to retrieve
            value: The value to set the option to
        """
        self._options.set(name, value)

    def setOptions(self, *args, **kwargs):
        """
        A method for setting/updating an objects options. (public)

        Usage:
           setOptions(sub0, sub1, ..., key0=value0, key1=value1, ...)
           Updates all sub-options with the provided key value pairs

           setOptions(key0=value0, key1=value1, ...)
           Updates the main options with the provided key,value pairs
        """
        # Sub-options case
        if args:
            for sub in args:
                if not self.options().hasOption(sub):
                    msg = "The supplied sub-option '{}' does not exist.".format(sub)
                    mooseutils.mooseError(msg)
                self._options.get(sub).update(**kwargs)

        # Main options case
        else:
            self._options.update(**kwargs)

    def printOption(self, name):
        """
        Print the title of the object with the setOptions key, value pair for the supplied option.

        Inputs:
            name[str]: The name of the option to display.
        """
        print '{}:{}={}'.format(self.title(), name, repr(self.getOption(name)))

    def printOptions(self):
        """
        Print a list of all available options for this object.
        """
        print self._options

    def printSetOptions(self):
        """
        Print python code for the 'setOptions' method.
        """
        output, sub_output = self._options.toScriptString()
        print 'setOptions({})'.format(', '.join(output))
        for key, value in sub_output.iteritems():
            print 'setOptions({}, {})'.format(key, ', '.join(repr(value)))
