#pylint: disable=missing-docstring
from MooseDocs import common
from MooseDocs.extensions import command

def make_extension(**kwargs):
    return ConfigExtension(**kwargs)

class ConfigExtension(command.CommandExtension):
    """
    Allows the configuration items of objects to be changes on a per-page basis.

    TODO: This should provide a means for updating configuration for all objects, including
          Extension, Translator, and Reader. Currently, this only works for the Renderer, which
          was the most immediate need.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self.local_config = dict()

    def reint(self):
        self.local_config.clear()

    def extend(self, reader, renderer):
        self.addCommand(ConfigRendererCommand())

    def preRender(self, root, config): #pylint: disable=unused-argument
        config.update(self.local_config)

class ConfigRendererCommand(command.CommandComponent):
    COMMAND = 'config'
    SUBCOMMAND = 'renderer'
    PARSE_SETTINGS = False

    def createToken(self, info, parent):
        defaults = self.translator.renderer.getConfig()
        known, _ = common.match_settings(defaults, info['settings'])
        self.extension.local_config = known
        return parent
