#pylint: disable=missing-docstring, no-self-use
from MooseDocs.base import components
from MooseDocs.extensions import command, materialicon
from MooseDocs.tree import tokens, html
from MooseDocs.tree.base import Property

def make_extension(**kwargs):
    return LayoutExtension(**kwargs)

class ColumnToken(tokens.Token):
    PROPERTIES = [Property('width', ptype=unicode, required=False)]

class RowToken(tokens.Token):
    pass

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
        self.requires(command, materialicon)
        self.addCommand(RowCommand())
        self.addCommand(ColumnCommand())

        renderer.add(ColumnToken, RenderColumnToken())
        renderer.add(RowToken, RenderRowToken())

class RowCommand(command.CommandComponent):
    COMMAND = 'row'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, info, parent):
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

    def createToken(self, info, parent):
        col = ColumnToken(parent, width=self.settings['width'], **self.attributes)

        icon = self.settings.get('icon', None)
        if icon:
            block = materialicon.IconBlockToken(col)
            h = tokens.Heading(block, level=2, class_='center brown-text')
            materialicon.IconToken(h, icon=unicode(icon))
            return block

        return col

class RenderRowToken(components.RenderComponent):
    def createHTML(self, token, parent):
        row = html.Tag(parent, 'div', class_='moose-row', **token.attributes)
        row.addStyle('display:flex')
        return row

    def createMaterialize(self, token, parent):
        row = html.Tag(parent, 'div', class_='row', **token.attributes)
        return row

    def createLatex(self, token, parent):
        pass

class RenderColumnToken(components.RenderComponent):
    def createHTML(self, token, parent):
        col = html.Tag(parent, 'div', class_='moose-column', **token.attributes)
        col.addStyle('flex:{};'.format(token.width))
        return col

    def createMaterialize(self, token, parent):
        col = html.Tag(parent, 'div', **token.attributes)
        col.addClass('col')
        return col

    def createLatex(self, token, parent):
        pass
