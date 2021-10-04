import os
from MooseDocs.base import components, LatexRenderer, HTMLRenderer, MarkdownReader
from MooseDocs.tree import tokens, html, latex
from MooseDocs.extensions import command, materialicon

def make_extension(**kwargs):
    return Alert2Extension(**kwargs)

Alert2Token = tokens.newToken('Alert2Token', brand='')

class Alert2Extension(command.CommandExtension):
    """
    Adds alert boxes (note, tip and warning) to display important information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['use-title-prefix'] = (True, "Enable/disable including the brand (e.g., ERROR) as " \
                                            "prefix for the alert title.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, Alert2Command())
        renderer.add('Alert2Token', RenderAlert2Token())

        if isinstance(renderer, HTMLRenderer):
            renderer.addCSS('alert_moose', "css/alert_moose.css")

class Alert2Command(command.CommandComponent):
    COMMAND = 'alert2'
    SUBCOMMAND = ('warning', 'note', 'tip')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page):
        brand = info['subcommand']
        return Alert2Token(parent, brand=brand)

class RenderAlert2Token(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'div', class_='alert2 {}'.format(token['brand']))

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'div', class_='alert2 {}'.format(token['brand']))

    def createLatex(self, parent, token, page):
        return parent
