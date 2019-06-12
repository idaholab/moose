#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from MooseDocs.base import components, renderers
from MooseDocs.common import exceptions
from MooseDocs.extensions import command
from MooseDocs.tree import tokens, html, latex

def make_extension(**kwargs):
    return StyleExtension(**kwargs)

StyleToken = tokens.newToken('StyleToken', halign='left', color=None, border=0, fontsize=None)

class StyleExtension(command.CommandExtension):
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, StyleCommand())
        renderer.add('StyleToken', RenderStyleToken())

        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage(u'xcolor')


class StyleCommand(command.CommandComponent):
    COMMAND = 'style'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['halign'] = (None, "The horizontal alignment ('center', 'left', or 'right')")
        settings['border'] = (None, "The size of the border in pixels")
        settings['color'] = (None, "Set the color of content.")
        settings['fontsize'] = (None, "Set the font size.")
        return settings

    def createToken(self, parent, info, page):
        return StyleToken(parent,
                          halign=self.settings['halign'],
                          border=self.settings['border'],
                          color=self.settings['color'],
                          fontsize=self.settings['fontsize'],
                          **self.attributes)

class RenderStyleToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        style = [token['style']]
        if token['halign']:
            if token['halign'] not in ('center', 'left', 'right'):
                msg = "The supplied string for 'halign' is '{}' but it must be " \
                      "'center', 'left', 'right'."
                raise exceptions.MooseDocsException(msg, token['halign'])

            style.append('text-align:{}'.format(token['halign']))
        if token['border']:
            style.append('border-width:{}px;border-style:solid'.format(token['border']))
        if token['color']:
            style.append('color:{}'.format(token['color']))
        if token['fontsize']:
            style.append('font-size:{}'.format(token['fontsize']))

        tag_type = 'span'
        if token.info.pattern in ('BlockInlineCommand', 'BlockBlockCommand'):
            tag_type = 'div'
        return html.Tag(parent, tag_type, token, style=';'.join(style))

    def createLatex(self, parent, token, page):
        master = parent

        if token['halign']:
            halign = token['halign']
            if halign != 'center':
                halign = 'flush{}'.format(halign)
            master = latex.Environment(master, halign)

        if token['border']:
            master = latex.Environment(master, 'fbox')

        if token['color']:
            master = latex.Brace(master)
            latex.Command(master, 'color', string=token['color'])

        return master
