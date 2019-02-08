#pylint: disable=missing-docstring,attribute-defined-outside-init
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs.base import components, LatexRenderer
from MooseDocs.tree import html, tokens, latex
from MooseDocs.extensions import command, core

def make_extension(**kwargs):
    return MaterialIconExtension(**kwargs)

Icon = tokens.newToken('Icon', icon=u'', faicon=u'')

class MaterialIconExtension(command.CommandExtension):
    "Adds ability to include material icons."""

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, IconCommand())
        renderer.add('Icon', RenderIcon())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('fontawesome')

class IconCommand(command.CommandComponent):
    COMMAND = 'icon'
    SUBCOMMAND = '*'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['faicon'] = (None, "When the LaTeX renderer is used, this will override the " \
                                    "supplied subcommand item. The name should exist in the " \
                                    "LaTeX fontawesome package.")
        return settings

    def createToken(self, parent, info, page):
        if info.pattern not in ('InlineCommand', 'OldInlineCommand'):
            core.Paragraph(parent)
        return Icon(parent, icon=info['subcommand'], faicon=self.settings['faicon'])

class RenderIcon(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        i = html.Tag(parent, 'i', token, string=token['icon'])
        i.addClass('material-icons')
        i.addClass('moose-inline-icon')

    def createLatex(self, parent, token, page):
        icon = token['faicon'] or token['icon']
        latex.Command(parent, 'faicon', string=icon, escape=False)
