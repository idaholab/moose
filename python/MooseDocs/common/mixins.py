"""
Contains base classes intended to be used internal to this module.
"""
import uuid
import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions

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

    def update(self, **kwargs):
        """
        Update the configuration with the supplied key-value pairs.
        """
        unknown = []
        for key, value in kwargs.iteritems():

            if key not in self.__config:
                unknown.append(key)
            else:
                self.__config[key] = (value, self.__config[key][1]) #TODO: type check???

        if unknown:
            msg = "The following config options were not found in the default config options for " \
                  "the {} object:"
            for key in unknown:
                msg += '\n{}{}'.format(' '*4, key)
            raise exceptions.MooseDocsException(msg.format(type(self)))

    def getConfig(self):
        """
        Return a dict() of the key-value pairs for the supplied configuration objects, this method
        removes the description.
        """
        return {key:value[0] for key, value in self.__config.iteritems()}

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

class TranslatorObject(object):
    """
    Class for objects that require a Translator object be created via an init method.
    """
    def __init__(self):
        self.__translator = None

    def init(self, translator):
        """
        Called by Translator object, this allows the objects to be
        created independently then passed into the translator, which then
        calls this method to provide access to translator for when the actual
        tokenize and render commands are called.
        """
        if self.initialized():
            msg = "The {} object has already been initialized, this method should not " \
                  "be called twice."
            raise MooseDocs.common.exceptions.MooseDocsException(msg, type(self))

        common.check_type('translator', translator, MooseDocs.base.translators.Translator)
        self.__translator = translator

    def initialized(self):
        """
        Returns True if the init method was called.
        """
        return self.__translator is not None

    @property
    def translator(self):
        """
        Returns the Translator object as property.
        """
        if self.__translator is None:
            msg = "The init() method of the {} object must be called prior to accessing this " \
                  "property."
            raise MooseDocs.common.exceptions.MooseDocsException(msg, type(self))
        return self.__translator

    def reinit(self):
        """
        Called by the Translator prior to converting, this allows for state to be reset when using
        livereload.
        """
        pass

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
        common.check_type("component", comp, MooseDocs.base.components.Component)
        self.__components.append(comp)
