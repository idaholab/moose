#pylint: disable=missing-docstring, no-self-use
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs.base import components
from MooseDocs.extensions import core, command, materialicon
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return LayoutExtension(**kwargs)

ColumnToken = tokens.newToken('ColumnToken', width=u'')
RowToken = tokens.newToken('RowToken')

class LayoutExtension(command.CommandExtension):
    """
    Adds ability to create row and column layouts.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['use-title-prefix'] = (True, "Enable/disable including the brand (e.g., ERROR) as " \
                                            "prefix for the alert title.")
        return config

    def extend(self, reader, renderer):
        self.requires(core, command, materialicon)
        self.addCommand(reader, RowCommand())
        self.addCommand(reader, ColumnCommand())

        renderer.add('ColumnToken', RenderColumnToken())
        renderer.add('RowToken', RenderRowToken())

class RowCommand(command.CommandComponent):
    COMMAND = 'row'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        return RowToken(parent, **self.attributes)


class ColumnCommand(command.CommandComponent):
    COMMAND = 'col'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['width'] = (None, "The default width of the column.")
        settings['icon'] = (None, "Material icon to place at top of column.")
        return settings

    def createToken(self, parent, info, page):
        col = ColumnToken(parent, width=self.settings['width'], **self.attributes)

        icon = self.settings.get('icon', None)
        if icon:
            block = materialicon.IconBlockToken(col)
            h = core.Heading(block, level=2, class_='center brown-text')
            materialicon.IconToken(h, icon=unicode(icon))
            return block

        return col

class RenderRowToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        row = html.Tag(parent, 'div', token)
        row.addClass('moose-row')
        row.addStyle('display:flex')
        return row

    def createMaterialize(self, parent, token, page):
        row = html.Tag(parent, 'div', token)
        row.addClass('row')
        return row

    def createLatex(self, parent, token, page):
        pass

class RenderColumnToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        col = html.Tag(parent, 'div', token)
        col.addStyle('flex:{};'.format(token['width']))
        col.addClass('moose-column')
        return col

    def createMaterialize(self, parent, token, page):
        col = html.Tag(parent, 'div', token)
        col.addClass('col')
        return col

    def createLatex(self, parent, token, page):
        pass
