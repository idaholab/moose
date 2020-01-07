#pylint: disable=missing-docstring
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
        self.__configurations = collections.defaultdict(dict)

    def initMetaData(self, page, meta):
        """Initialize the page as active."""
        meta.initData('active', True)

    def postRead(self, content, page, meta):
        """Updates configuration items."""
        if content:
            for match in command.BlockInlineCommand.RE.finditer(content):
                if match.group('command') == 'config':
                    subcommand = match.group('subcommand')
                    _, settings = common.match_settings(dict(), match.group('settings'))
                    if subcommand == 'disable':
                        self.__configPageDisable(page, meta, settings)
                    else:
                        self.__configurations[page.uid][subcommand] = settings

    @staticmethod
    def __configPageDisable(page, meta, settings):
        """Activate/deactivate based on extension."""

        _, ext = os.path.splitext(page.destination)
        extensions = eval(settings.get('extensions'))
        if extensions and ext in extensions:
            meta.setData('active', False)

    def preTokenize(self, ast, page, meta, reader):
        for key, value in self.__configurations[page.uid].items():
            self.translator.updateConfiguration(key, **value)

    def postTokenize(self, ast, page, meta, reader):
        self.translator.resetConfigurations()

    def preRender(self, result, page, meta, renderer):
        for key, value in self.__configurations[page.uid].items():
            self.translator.updateConfiguration(key, **value)

    def postWrite(self, *args):
        self.translator.resetConfigurations()

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, ConfigCommand())
        self.addCommand(reader, ConfigPageActiveCommand())

class ConfigCommand(command.CommandComponent):
    """This does nothing but serves to hide the command syntax from outputting."""
    COMMAND = 'config'
    SUBCOMMAND = '*'
    PARSE_SETTINGS = False

    def createToken(self, parent, info, page):
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

    def createToken(self, parent, info, page):
        return parent
