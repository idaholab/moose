#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import collections
from .. import common
from . import command

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

    def postRead(self, page, content):
        """Updates configuration items."""
        if content:
            for match in command.BlockInlineCommand.RE.finditer(content):
                if match.group('command') == 'config':
                    subcommand = match.group('subcommand')
                    _, settings = common.match_settings(dict(), match.group('settings'))
                    if subcommand == 'disable':
                        self.__configPageDisable(page, settings)
                    else:
                        page['__{}__'.format(subcommand)].update(settings)

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, ConfigCommand())
        self.addCommand(reader, ConfigPageActiveCommand())

    @staticmethod
    def __configPageDisable(page, settings):
        """Activate/deactivate based on extension."""
        _, ext = os.path.splitext(page.destination)
        extensions = eval(settings.get('extensions'))
        if extensions and ext in extensions:
            page['active'] = False

class ConfigCommand(command.CommandComponent):
    """This does nothing but serves to hide the command syntax from outputting."""
    COMMAND = 'config'
    SUBCOMMAND = '*'
    PARSE_SETTINGS = False

    def createToken(self, parent, info, page, settings):
        return parent

class ConfigPageActiveCommand(command.CommandComponent):
    """This does nothing but serves to hide the command syntax from outputting."""
    COMMAND = 'config'
    SUBCOMMAND = 'disable'
    PARSE_SETTINGS = False

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['extensions'] = ([], "If the output extension matches the page is disabled from " \
                                      "translation.")
        return settings

    def createToken(self, parent, info, page, settings):
        return parent
