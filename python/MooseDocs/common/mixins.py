#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Contains base classes intended to be used internal to this module.
"""
import uuid
import copy
import MooseDocs
from ..common import exceptions

#: A value for allowing ConfigObject.get method to work with a default of None
UNSET = uuid.uuid4()

def assertInitialized(object, method):
    """Error if the object is not set, and which method to call to do so."""
    if object is None:
        msg = "The {}() method must be called prior to accessing this property."
        raise MooseDocs.common.exceptions.MooseDocsException(msg, method)

class ConfigObject(object):
    """
    Base class for objects that contain configure options.
    """
    #: A value for allowing ConfigObject.get method to work with a default of None
    UNSET = UNSET

    @staticmethod
    def defaultConfig():
        """
        Return the default configuration for this object. The configuration should be a dict of
        tuples, where the tuple contains the default value and the description.
        """
        return dict()

    def __init__(self, *args, **kwargs):
        config = self.defaultConfig()
        if not isinstance(config, dict):
            msg = "The return type from 'defaultConfig' must be a 'dict', but a {} was provided."
            raise exceptions.MooseDocsException(msg, type(config))
        self.__config = {key:value[0] for key, value in config.items()}
        self.update(**kwargs)
        self.__name = args[0] if args else None

    @property
    def name(self):
        """Return the name of the extension."""
        return self.__name

    def initConfig(self, page, *keys):
        """Initialize page config for the supplied keys"""
        if self.name is None:
            raise exceptions.MooseDocsException('Page level config items are not available for this object.')

        for key in keys:
            self.setConfig(page, key, self.get(key))

    def getConfig(self, page, key):
        """Return a per page config item"""
        if self.name is None:
            raise exceptions.MooseDocsException('Page level config items are not available for this object.')
        return page['__{}__'.format(self.name)][key]

    def setConfig(self, page, key, value):
        """Set a per page config item"""
        if self.name is None:
            raise exceptions.MooseDocsException('Page level config items are not available for this object.')
        page['__{}__'.format(self.name)][key] = value

    def update(self, **kwargs):
        """
        Update the configuration with the supplied key-value pairs.
        """
        error_on_unknown = kwargs.pop('error_on_unknown', True)

        unknown = []
        for key, value in kwargs.items():
            if key not in self.__config:
                unknown.append(key)
            else:
                self.__config[key] = value#(value, self.__config[key][1]) #TODO: type check???

        if unknown and error_on_unknown:
            msg = "The following config options were not found in the default config options for " \
                  "the {} object:"
            for key in unknown:
                msg += '\n{}{}'.format(' '*4, key)
            raise exceptions.MooseDocsException(msg.format(type(self)))

    def keys(self):
        """
        Return the available configuration items.
        """
        return self.__config.keys()

    def __getitem__(self, name):
        """
        Return a configuration value by name using the [] operator.
        """
        return self.get(name)

    def get(self, name, default=UNSET):
        """
        Return a configuration value by name, with an optional default.
        """
        if (default is not UNSET) and (name not in self.__config):
            return default
        else:
            return self.__config[name]

    def __contains__(self, name):
        """
        Check for config item.
        """
        return name in self.__config

class ReaderObject(object):
    """
    Basic functions for objects that have a Reader object.
    """
    def __init__(self):
        self.__reader = None

    def setReader(self, reader):
        """Initialize the class with the Reader object."""
        self.__reader = reader

    @property
    def reader(self):
        """Return the Reader object."""
        assertInitialized(self.__reader, 'setReader')
        return self.__reader

class RendererObject(object):
    """
    Basic functions for objects that have a Renderer object.
    """
    def __init__(self):
        self.__renderer = None

    def setRenderer(self, renderer):
        """Initialize the class with the Renderer object."""
        self.__renderer = renderer

    @property
    def renderer(self):
        """Return the Renderer object."""
        assertInitialized(self.__renderer, 'setRenderer')
        return self.__renderer

class ComponentObject(object):
    """
    Class for objects that require a list of components (e.g., Reader and Renderers).
    """
    def __init__(self):
        self.__components = []
        self.__translator = None

    @property
    def translator(self):
        """Return the translator instance."""
        assertInitialized(self.__translator, 'setTranslator')
        return self.__translator

    @property
    def components(self):
        """Return the list of Component objects."""
        return self.__components

    def setTranslator(self, translator):
        """Method called by Translator to allow components access to it."""
        self.__translator = translator

    def addComponent(self, comp):
        """Add a Component object."""
        self.__components.append(comp)

class TranslatorObject(object):
    """
    Mixin for accessing translator (e.g., Extensions and Components).
    """
    def __init__(self):
        self.__translator = None

    def setTranslator(self, translator):
        """
        Method called by Translator to allow find methods to operate.
        """
        self.__translator = translator

    @property
    def translator(self):
        """Return the translator instance."""
        self.assertInitialized()
        return self.__translator

    def assertInitialized(self):
        """Error if the translator is not set."""
        assertInitialized(self.__translator, 'setTranslator')
