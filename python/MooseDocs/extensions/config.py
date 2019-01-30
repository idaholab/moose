#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
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

    def postRead(self, content, page, meta):
        """Updates configuration items."""
        if content:
            for match in command.BlockInlineCommand.RE.finditer(content):
                if match.group('command') == 'config':
                    subcommand = match.group('subcommand')
                    _, settings = common.match_settings(dict(), match.group('settings'))
                    self.__configurations[page.uid][subcommand] = settings

    def preTokenize(self, ast, page, meta, reader):
        for key, value in self.__configurations[page.uid].iteritems():
            self.translator.updateConfiguration(key, **value)

    def postTokenize(self, ast, page, meta, reader):
        self.translator.resetConfigurations()

    def preRender(self, result, page, meta, renderer):
        for key, value in self.__configurations[page.uid].iteritems():
            self.translator.updateConfiguration(key, **value)

    def postWrite(self, *args):
        self.translator.resetConfigurations()

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, ConfigRendererCommand())

class ConfigRendererCommand(command.CommandComponent):
    """This does nothing but serves to hide the command syntax from outputting."""
    COMMAND = 'config'
    SUBCOMMAND = '*'
    PARSE_SETTINGS = False

    def createToken(self, parent, info, page):
        return parent
