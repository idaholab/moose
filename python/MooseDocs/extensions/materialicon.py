#pylint: disable=missing-docstring,attribute-defined-outside-init
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs.base import components
from MooseDocs.tree import html, tokens
from MooseDocs.extensions import command

def make_extension(**kwargs):
    return MaterialIconExtension(**kwargs)

IconBlockToken = tokens.newToken('IconBlockToken')
IconToken = tokens.newToken('IconToken', icon=u'')

class MaterialIconExtension(command.CommandExtension):
    "Adds ability to include material icons."""

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, IconCommand())
        renderer.add('IconToken', RenderIconToken())
        renderer.add('IconBlockToken', RenderIconBlockToken())

class IconCommand(command.CommandComponent):
    COMMAND = 'icon'
    SUBCOMMAND = '*'

    def createToken(self, parent, info, page):
        return IconToken(parent, icon=info['subcommand'])

class RenderIconToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        i = html.Tag(parent, 'i', class_='material-icons moose-inline-icon', **token.attributes)
        html.String(i, content=token['icon'], hide=True)

    def createLatex(self, parent, token, page):
        pass

class RenderIconBlockToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        div = html.Tag(parent, 'div', class_='icon-block')
        return div

    def createLatex(self, parent, token, page):
        pass
