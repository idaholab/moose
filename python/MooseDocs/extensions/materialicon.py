#pylint: disable=missing-docstring,attribute-defined-outside-init
from MooseDocs.base import components
from MooseDocs.tree import html, tokens

def make_extension(**kwargs):
    return MaterialIconExtension(**kwargs)

class IconBlockToken(tokens.Token):
    pass

class IconToken(tokens.Token):
    PROPERTIES = [tokens.Property('icon', ptype=unicode, required=True)]

class MaterialIconExtension(components.Extension):
    "Adds ability to include material icons."""

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        renderer.add(IconToken, RenderIconToken())
        renderer.add(IconBlockToken, RenderIconBlockToken())

class RenderIconToken(components.RenderComponent):
    def createHTML(self, token, parent):
        pass

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use,unused-argument
        i = html.Tag(parent, 'i', class_='material-icons', **token.attributes)
        html.String(i, content=token.icon, hide=True)

    def createLatex(self, token, parent):
        pass

class RenderIconBlockToken(components.RenderComponent):
    def createHTML(self, token, parent):
        pass

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use,unused-argument
        div = html.Tag(parent, 'div', class_='icon-block')
        return div

    def createLatex(self, token, parent):
        pass
