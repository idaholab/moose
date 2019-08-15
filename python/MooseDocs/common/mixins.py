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
from MooseDocs.common import exceptions, check_type

#: A value for allowing ConfigObject.get method to work with a default of None
UNSET = uuid.uuid4()

class ConfigObject(object):
    """
    Base class for objects that contain configure options.
    """
    @staticmethod
    def defaultConfig():
        """
        Return the default configuration for this object. The configuration should be a dict of
        tuples, where the tuple contains the default value and the description.
        """
        return dict()

    def __init__(self, **kwargs):
        self.__config = self.defaultConfig()
        if not isinstance(self.__config, dict):
            msg = "The return type from 'defaultConfig' must be a 'dict', but a {} was provided."
            raise exceptions.MooseDocsException(msg, type(self.__config))
        self.update(**kwargs)

        # Stores the configuration that was established upon object creation, this allows the
        # config extension to mess with configuration items during execute but then restore
        # prior to processing the next page.
        self.__initial_config = copy.copy(self.__config)

    def update(self, **kwargs):
        """
        Update the configuration with the supplied key-value pairs.
        """
        set_initial = kwargs.pop('set_initial', False)
        error_on_unknown = kwargs.pop('error_on_unknown', True)

        unknown = []
        for key, value in kwargs.items():

            if key not in self.__config:
                unknown.append(key)
            else:
                self.__config[key] = (value, self.__config[key][1]) #TODO: type check???

        if unknown and error_on_unknown:
            msg = "The following config options were not found in the default config options for " \
                  "the {} object:"
            for key in unknown:
                msg += '\n{}{}'.format(' '*4, key)
            raise exceptions.MooseDocsException(msg.format(type(self)))

        if set_initial:
            self.__initial_config = copy.copy(self.__config)

    def resetConfig(self):
        """
        Reset configuration to original state.
        """
        self.__config = copy.copy(self.__initial_config)

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
            return self.__config[name][0]

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
        check_type('reader', reader, MooseDocs.base.readers.Reader)
        self.__reader = reader

    @property
    def reader(self):
        """Return the Reader object."""
        self.assertInitialized()
        return self.__reader

    def assertInitialized(self):
        """Error if the reader is not set."""
        if self.__reader is None:
            msg = "The setTranslator() method must be called prior to accessing this property."
            raise MooseDocs.common.exceptions.MooseDocsException(msg)

class RendererObject(object):
    """
    Basic functions for objects that have a Renderer object.
    """
    def __init__(self):
        self.__renderer = None

    def setRenderer(self, renderer):
        """Initialize the class with the Renderer object."""
        check_type('renderer', renderer, MooseDocs.base.renderers.Renderer)
        self.__renderer = renderer

    @property
    def renderer(self):
        """Return the Renderer object."""
        return self.__renderer

    def assertInitialized(self):
        """Error if the renderer has not been set."""
        if self.__renderer is None:
            msg = "The setRenderer() method must be called prior to accessing this property."
            raise MooseDocs.common.exceptions.MooseDocsException(msg, type(self))

class ComponentObject(object):
    """
    Class for objects that require a list of components (e.g., Reader and Renderers).
    """
    def __init__(self):
        self.__components = []

    @property
    def components(self):
        """
        Return the list of Component objects.
        """
        return self.__components

    def addComponent(self, comp):
        """
        Add a Component object.
        """
        check_type("component", comp, MooseDocs.base.components.Component)
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
        check_type('translator', translator, MooseDocs.base.translators.Translator)
        self.__translator = translator

    @property
    def translator(self):
        """Return the translator instance."""
        self.assertInitialized()
        return self.__translator

    def assertInitialized(self):
        """Error if the translator is not set."""
        if self.__translator is None:
            msg = "The setTranslator() method must be called prior to accessing this property."
            raise MooseDocs.common.exceptions.MooseDocsException(msg)
