#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from ..common import exceptions
from ..base import components
from ..tree import tokens, html, latex
from . import core, command, materialicon

def make_extension(**kwargs):
    return LayoutExtension(**kwargs)

ColumnToken = tokens.newToken('ColumnToken', width='', small=12, medium=12, large=12)
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

    def createToken(self, parent, info, page, settings):
        return RowToken(parent, **self.attributes(settings))


class ColumnCommand(command.CommandComponent):
    COMMAND = 'col'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['width'] = (None, "The default width of the column (HTML output only).")
        settings['icon'] = (None, "Material icon to place at top of column.")
        settings['small'] = (12, "The number of columns for small displays (1-12).")
        settings['medium'] = (12, "The number of columns for medium displays (1-12), " \
                                  "this is used by the LaTeX output for determining the number " \
                                  "of columns.")
        settings['large'] = (12, "The number of columns for large displays (1-12).")
        return settings

    def createToken(self, parent, info, page, settings):

        sml = []
        for s in ['small', 'medium', 'large']:
            sml.append(int(settings[s]))
            if sml[-1] < 1 or sml[-1] > 12:
                msg = "The '{}' setting must be an integer between 1 and 12."
                raise exceptions.MooseDocsException(msg, s)

        col = ColumnToken(parent,
                          width=settings['width'],
                          small=sml[0],
                          medium=sml[1],
                          large=sml[2],
                          **self.attributes(settings))

        icon = settings.get('icon', None)
        if icon:
            materialicon.Icon(col, icon=str(icon), class_='moose-col-icon')

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
        return parent

class RenderColumnToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        col = html.Tag(parent, 'div', token)
        col.addStyle('flex:{};'.format(token['width']))
        col.addClass('moose-column')
        return col

    def createMaterialize(self, parent, token, page):
        col = html.Tag(parent, 'div', token)
        col.addClass('col')
        col.addClass('s{}'.format(token['small']))
        col.addClass('m{}'.format(token['medium']))
        col.addClass('l{}'.format(token['large']))
        return col

    def createLatex(self, parent, token, page):
        pad = len(token.parent)*0.01
        width = '{}\\textwidth'.format(token['medium']/12. - pad)
        env = latex.Environment(parent, 'minipage',
                                args=[latex.Bracket(string='t'),
                                      latex.Brace(string=width, escape=False)])
        if token is not token.parent.children[-1]:
            latex.Command(parent, 'hfill')
        return env
