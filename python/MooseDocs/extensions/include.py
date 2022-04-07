#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .. import common
from ..base import RevealRenderer, Translator
from . import command

def make_extension(**kwargs):
    return IncludeExtension(**kwargs)

class IncludeExtension(command.CommandExtension):
    """Enables the !include command for including files in other files."""

    def extend(self, reader, renderer):
        self.requires(command)

        if isinstance(renderer, RevealRenderer):
            self.addCommand(reader, IncludeSlides())
        else:
            self.addCommand(reader, IncludeCommand())

    def preRead(self, page):
        page['dependencies'] = set()

class IncludeCommand(command.CommandComponent):
    COMMAND = 'include'
    SUBCOMMAND = 'md' #TODO: get this from the reader inside the __init__ method.

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(common.extractContentSettings())
        settings['optional'] = (False, "Toggle the include as optional when the file doesn't exist.")
        return settings

    def createToken(self, parent, info, page, settings):
        """
        Tokenize the included content and create dependency between pages.
        """
        include_page = self.translator.findPage(info['subcommand'], throw_on_zero=not settings['optional'])
        if include_page is None and settings['optional']:
            return parent
        content, line = common.extractContent(self.reader.read(include_page), settings)

        self.reader.tokenize(parent, content, page, line=line)
        page['dependencies'].add(include_page.uid)
        return parent

class IncludeSlides(IncludeCommand):

    @staticmethod
    def defaultSettings():
        settings = IncludeCommand.defaultSettings()
        settings['vertical'] = (True, "Included content will be included as vertical slides.")
        return settings

    def createToken(self, parent, info, page, settings):
        idx = len(parent.children)
        IncludeCommand.createToken(self, parent, info, page, settings)

        if settings['vertical']:
            for child in parent.children[idx:]:
                if child.name == 'Section':
                    child.name = 'SubSection'

        return parent
