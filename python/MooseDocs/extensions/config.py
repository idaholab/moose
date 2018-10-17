#pylint: disable=missing-docstring
from MooseDocs import common
from MooseDocs.extensions import command

def make_extension(**kwargs):
    return ConfigExtension(**kwargs)

class ConfigExtension(command.CommandExtension):
    """
    Allows the configuration items of objects to be changes on a per-page basis.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

    def postRead(self, content, page, meta):
        """Updates configuration items."""
        if content:
            for match in command.BlockInlineCommand.RE.finditer(content):
                if match.group('command') == 'config':
                    subcommand = match.group('subcommand')
                    meta.initData(subcommand, dict)
                    _, settings = common.match_settings(dict(), match.group('settings'))
                    meta.getData(subcommand).update(**settings)

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, ConfigRendererCommand())

class ConfigRendererCommand(command.CommandComponent):
    """This does nothing but that serves to hide the command syntax from outputting."""
    COMMAND = 'config'
    SUBCOMMAND = '*'
    PARSE_SETTINGS = False

    def createToken(self, parent, info, page):
        return parent
