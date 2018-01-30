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
    def getOptions():
        """
        All object should define a static getOptions method to add new key, value options. (public)
        """
        opt = utils.Options()
        opt.add('debug', False, "Enable/disable debug messaging.")
        return opt

    def __init__(self, **kwargs):
        super(ChiggerObject, self).__init__()
        self._options = getattr(self.__class__, 'getOptions')()
        self.__initial_options = kwargs
        self.__needs_initialize = True
        self.__needs_update = True

    def options(self):
        """
        Return the utils.Options object for this class.
        """
        return self._options

    def needsUpdate(self):
        """
        Return True if the object requires an Update method call. (public)
        """
        mooseutils.mooseDebug("{}.needsUpdate() = {}".format(self.__class__.__name__,
                                                             self.__needs_update))
        return self.__needs_update

    def needsInitialize(self):
        """
        Return True if the object requires an _initialize method call. (public)
        """
        mooseutils.mooseDebug("{}.needsInitialize() = {}".format(self.__class__.__name__,
                                                                 self.__needs_initialize))
        return self.__needs_initialize

    def update(self, initialize=True, **kwargs):
        """
        Update method should contain calls to underlying vtk objects. (public)

        Inputs:
            initialize[bool]: When True the initialize() method will be called, but only if needed.
        """
        if self.__needs_initialize and initialize:
            self.initialize()
        mooseutils.mooseDebug("{}.update()".format(self.__class__.__name__))
        self.setOptions(**kwargs)
        self.setNeedsUpdate(False)

    def isOptionValid(self, name):
        """
        Test if the given option is valid (i.e., not None). (public)
        """
        return self._options.isOptionValid(name)

    def getOption(self, name):
        """
        Return the value of an option. (public)

        Inputs:
            name[str]: The name of the option to retrieve
        """
        return self._options[name]

    def setOption(self, name, value):
        """
        Set single option. (public)

        Inputs:
            name[str]: The name of the option to retrieve
            value: The value to set the option to
        """
        changed = (self._options[name] != value)
        if changed:
            self._options[name] = value
            self.setNeedsUpdate(True)

    def setOptions(self, *args, **kwargs):
        """
        A method for setting/updating an objects options. (public)

        Usage:
           setOptions(sub0, sub1, ..., key0=value0, key1=value1, ...)
           Updates all suboptions with the provided key value pairs

           setOptions(key0=value0, key1=value1, ...)
           Updates the main options with the provided key,value pairs
        """

        # Sub-options case
        changed = [self.needsUpdate()] # default changed status to the current status
        if len(args) > 0:
            for sub in args:
                if (self._options.hasOption(sub)) and isinstance(self.getOption(sub),
                                                                 utils.Options):
                    changed.append(self._options[sub].update(**kwargs))
                elif isinstance(sub, utils.Options):
                    changed.append(self._options.update(sub))
        # Main options case
        else:
            changed.append(self._options.update(**kwargs))

        changed = any(changed)
        self.setNeedsUpdate(changed)
        return changed

    def updateOptions(self, *args):
        """
        Apply the supplied option objects to this object and the contained ChiggerFilterSourceBase
        objects. (override)

        Inputs:
            see ChiggerResultBase
        """
        changed = [self.needsUpdate()]
        for sub in args:
            changed.append(self._options.update(sub))
        changed = any(changed)
        self.setNeedsUpdate(changed)
        return changed

    def reset(self):
        """
        Reset initialization flag, so the _initialize method will be called again on next update.
        """
        self.__needs_initialize = True

    def initialize(self):
        """
        Initialize method that runs once when update() is first called. (protected)
        """
        mooseutils.mooseDebug("{}.initialize()".format(self.__class__.__name__))
        self.__needs_initialize = False
        self._setInitialOptions()

    def _setInitialOptions(self):
        """
        Method for applying the options passed to constructor, this is called by the
        initialize() method automatically.
        """
        if self.__initial_options:
            self.setOptions(**self.__initial_options)
            self.__initial_options = None

    def setNeedsUpdate(self, value):
        """
        Set the value of the need update flag. (protected)

        Inputs:
            value[bool]: The value for the update flag.
        """
        mooseutils.mooseDebug("{}.setNeedsUpdate({})".format(self.__class__.__name__, value))
        self.__needs_update = value

    def _setNeedsInitialize(self, value):
        """
        Set the initialize flag for the _initialize method. (protected)
        """
        mooseutils.mooseDebug("{}._setNeedsInitialize({})".format(self.__class__.__name__, value))
        self.__needs_initialize = value

    def checkUpdateState(self):
        """
        Checks if the object needs update and performs updated, if needed.
        """
        if self.needsUpdate():
            self.update()
