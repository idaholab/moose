#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import MooseDocs
from MooseDocs.base import components
from MooseDocs.extensions import command
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return AlertExtension(**kwargs)

AlertToken = tokens.newToken('AlertToken', brand=u'')
AlertTitle = tokens.newToken('AlertTitle', brand=u'', prefix=True)
AlertContent = tokens.newToken('AlertContent', brand=u'', icon=True)

class AlertExtension(command.CommandExtension):
    """
    Adds alert boxes (note, error, warning, and construction) to display important information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['use-title-prefix'] = (True, "Enable/disable including the brand (e.g., ERROR) as " \
                                            "prefix for the alert title.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, AlertCommand())
        renderer.add('AlertToken', RenderAlertToken())
        renderer.add('AlertTitle', RenderAlertTitle())
        renderer.add('AlertContent', RenderAlertContent())

class AlertCommand(command.CommandComponent):
    COMMAND = 'alert'
    SUBCOMMAND = ('error', 'warning', 'note', 'construction')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['title'] = (None, "The optional alert title.")
        settings['prefix'] = (None, "Enable/disable the title being prefixed with the alert brand.")
        settings['icon'] = (True, "Enable/disable the icon.")
        return settings

    def createToken(self, parent, info, page):
        title = self.settings.pop('title', None)
        brand = info['subcommand']

        if self.settings['prefix'] is not None:
            prefix = self.settings['prefix']
        else:
            prefix = self.extension.get('use-title-prefix', True)

        alert_token = AlertToken(parent, brand=brand)
        title_token = AlertTitle(alert_token, brand=brand, prefix=prefix)

        if title:
            self.reader.tokenize(title_token, title, page, MooseDocs.INLINE)

        return AlertContent(alert_token, brand=brand, icon=self.settings['icon'])


class RenderAlertToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div', class_='moose-alert moose-alert-{}'.format(token['brand']))
        content = html.Tag(div, 'div', class_='moose-alert-content')
        return content

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent,
                        'div',
                        class_='card moose-alert moose-alert-{}'.format(token['brand']))

    def createLatex(self, parent, token, page):
        pass

class RenderAlertContent(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'p')

    def createMaterialize(self, parent, token, page):

        card_content = html.Tag(parent, 'div', class_='card-content')
        content = html.Tag(card_content, 'div', class_='moose-alert-content')

        if token['icon'] and (token['brand'] == 'construction'):
            src = os.path.relpath('media/framework/under-construction.gif',
                                  os.path.dirname(page.local))
            html.Tag(content, 'img', class_='moose-alert-construction-img', src=src)

        return html.Tag(content, 'p')

    def createLatex(self, parent, token, page):
        pass


class RenderAlertTitle(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'p')

    def createMaterialize(self, parent, token, page):

        title = html.Tag(parent, 'div', class_='card-title moose-alert-title')
        if token.get('prefix'):
            brand = token['brand']
            if brand == u'construction':
                brand = u'under construction'
            prefix = html.Tag(title, 'span', string=brand, class_='moose-alert-title-brand')
            if token.children:
                html.String(prefix, content=u':')

        return title

    def createLatex(self, parent, token, page):
        pass
