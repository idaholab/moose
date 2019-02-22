#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs.base import components
from MooseDocs.extensions import command
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return StyleExtension(**kwargs)

StyleToken = tokens.newToken('StyleToken', halign='left', border=0)

class StyleExtension(command.CommandExtension):
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, StyleCommand())
        renderer.add('StyleToken', RenderStyleToken())

class StyleCommand(command.CommandComponent):
    COMMAND = 'style'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['halign'] = (None, "The horizontal alignment ('center', 'left', or 'right')")
        settings['border'] = (None, "The size of the border in pixels")
        return settings

    def createToken(self, parent, info, page):
        return StyleToken(parent,
                          halign=self.settings['halign'],
                          border=self.settings['border'],
                          **self.attributes)

class RenderStyleToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        style = [token['style']]
        if token['halign']:
            style.append('text-align:{}'.format(token['halign']))
        if token['border']:
            style.append('border-width:{}px;border-style:solid'.format(token['border']))
        return html.Tag(parent, 'div', token, style=';'.join(style))

    def createLatex(self, parent, token, page):
        pass
