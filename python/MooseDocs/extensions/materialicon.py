#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from ..base import components, LatexRenderer
from ..tree import html, tokens, latex
from . import command, core

def make_extension(**kwargs):
    return MaterialIconExtension(**kwargs)

Icon = tokens.newToken('Icon', icon='', tight=False, faicon='')

class MaterialIconExtension(command.CommandExtension):
    "Adds ability to include material icons."""

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
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
        settings['tight'] = (False, "Use the same font size and line height of the parent element.")
        settings['faicon'] = (None, "When the LaTeX renderer is used, this will override the " \
                                    "supplied subcommand item. The name should exist in the " \
                                    "LaTeX fontawesome package.")
        return settings

    def createToken(self, parent, info, page, settings):
        if info.pattern not in ('InlineCommand', 'OldInlineCommand', 'OlderInlineCommand'):
            core.Paragraph(parent)
        return Icon(parent, icon=info['subcommand'],
                    tight=settings['tight'],
                    faicon=settings['faicon'])

class RenderIcon(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):
        i = html.Tag(parent, 'i', token, string=token['icon'])
        i.addClass('material-icons')
        i.addClass('moose-inline-icon')
        if token['tight']:
            i.addClass('moose-tight-inline-icon')

    def createLatex(self, parent, token, page):
        icon = token['faicon'] or token['icon']
        latex.Command(parent, 'faicon', string=icon, escape=False)
