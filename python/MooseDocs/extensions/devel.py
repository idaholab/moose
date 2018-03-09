#pylint: disable=missing-docstring
import importlib

from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import floats, command, table
from MooseDocs.tree import html, tokens
from MooseDocs.tree.base import Property

def make_extension(**kwargs):
    return DevelExtension(**kwargs)

class ExampleToken(tokens.Token):
    PROPERTIES = [Property("data", ptype=unicode, required=True)]

class DevelExtension(command.CommandExtension):
    """
    Adds features useful for MooseDocs developers such as example blocks and settings tables.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['test'] = (None, "A configuration item for testing purposes, it does nothing.")
        return config

    def extend(self, reader, renderer):
        self.requires(floats)

        self.addCommand(Example())
        self.addCommand(ComponentSettings())

        renderer.add(ExampleToken, RenderExampleToken())

class Example(command.CommandComponent):
    COMMAND = 'devel'
    SUBCOMMAND = 'example'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the code specification example.")
        settings['prefix'] = (u'Example', "The caption prefix (e.g., Example).")
        return settings

    def createToken(self, match, parent):

        master = floats.Float(parent, **self.attributes)
        caption = floats.Caption(master, prefix=self.settings['prefix'], key=self.attributes['id'])

        grammer = self.reader.lexer.grammer('inline')
        self.reader.lexer.tokenize(caption, grammer, unicode(self.settings['caption']), match.line)

        data = match['block'] if 'block' in match else match['inline']
        example = ExampleToken(master, data=data)
        return example

class ComponentSettings(command.CommandComponent):
    COMMAND = 'devel'
    SUBCOMMAND = 'settings'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['module'] = (None, "The name of the module containing the object.")
        settings['object'] = (None, "The name of the object to import from the 'module'.")
        settings['caption'] = (None, "The caption to use for the settings table created.")
        settings['prefix'] = (u'Table', "The caption prefix (e.g., Table).")

        return settings

    def createToken(self, match, parent):
        if self.settings['module'] is None:
            raise exceptions.TokenizeException("The 'module' setting is required.")

        if self.settings['object'] is None:
            raise exceptions.TokenizeException("The 'object' setting is required.")

        master = floats.Float(parent, **self.attributes)

        if self.settings['caption']:
            caption = floats.Caption(master, prefix=self.settings['prefix'],
                                     key=self.attributes['id'])
            grammer = self.reader.lexer.grammer('inline')
            self.reader.lexer.tokenize(caption, grammer, self.settings['caption'], match.line)

        try:
            mod = importlib.import_module(self.settings['module'])
        except ImportError:
            msg = "Unable to load the '{}' module."
            raise exceptions.TokenizeException(msg, self.settings['module'])

        try:
            obj = getattr(mod, self.settings['object'])
        except AttributeError:
            msg = "Unable to load the '{}' attribute from the '{}' module."
            raise exceptions.TokenizeException(msg, self.settings['object'],
                                               self.settings['module'])

        if hasattr(obj, 'defaultSettings'):
            settings = obj.defaultSettings()
        elif hasattr(obj, 'defaultConfig'):
            settings = obj.defaultConfig()
        else:
            msg = "The '{}' object in the '{}' module does not have a 'defaultSettings' or "\
                  "'defaultConfig' method."
            raise exceptions.TokenizeException(msg, mod, obj)

        rows = [[key, value[0], value[1]] for key, value in settings.iteritems()]
        tbl = table.builder(rows, headings=[u'Key', u'Default', u'Description'])
        tbl.parent = master
        return master

class RenderExampleToken(components.RenderComponent):

    def createHTML(self, token, parent):
        div = html.Tag(parent, 'div', class_='moose-example')
        left = html.Tag(div, 'div', class_='moose-example-code')
        ast = tokens.Code(left, code=token.data)
        self.translator.renderer.process(left, ast)
        html.Tag(div, 'div', class_='moose-example-rendered')

    def createMaterialize(self, token, parent):
        div = html.Tag(parent, 'div', class_='row card-content')
        left = html.Tag(div, 'div', class_='moose-example-code col s12 m12 l12')
        ast = tokens.Code(left, code=token.data)
        self.translator.renderer.process(left, ast)
        right = html.Tag(div, 'div', class_='moose-example-rendered col s12 m12 l12')
        return right
