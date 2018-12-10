#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs import common
from MooseDocs.extensions import command

def make_extension(**kwargs):
    return IncludeExtension(**kwargs)

class IncludeExtension(command.CommandExtension):
    """Enables the !include command for including files in other files."""

    def __init__(self, *args, **kwargs):
        super(IncludeExtension, self).__init__(*args, **kwargs)
        self.__dependencies = set()

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, IncludeCommand())

    def initMetaData(self, page, meta):
        meta.initData('dependencies', set())

    def postTokenize(self, ast, page, meta, reader):
        meta.getData('dependencies').update(self.__dependencies)
        self.__dependencies.clear()

    def addDependency(self, page):
        self.__dependencies.add(page.uid)

class IncludeCommand(command.CommandComponent):
    COMMAND = 'include'
    SUBCOMMAND = 'md' #TODO: get this from the reader inside the __init__ method.

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(common.extractContentSettings())
        return settings

    def createToken(self, parent, info, page):
        """
        Tokenize the included content and create dependency between pages.
        """
        include_page = self.translator.findPage(info['subcommand'])
        content, line = common.extractContent(self.reader.read(include_page), self.settings)
        self.reader.tokenize(parent, content, page, line=line)
        self.extension.addDependency(include_page)
        return parent
