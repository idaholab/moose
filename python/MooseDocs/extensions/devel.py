#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import importlib
import uuid
from ..base import components, LatexRenderer, HTMLRenderer
from ..common import exceptions
from ..tree import tokens, latex, html
from . import core, floats, command, table

def make_extension(**kwargs):
    return DevelExtension(**kwargs)

Example = tokens.newToken('Example')

EXAMPLE_LATEX = """
\\newtcolorbox
[auto counter,number within=chapter]{example}[2][]{%
skin=bicolor,colback=black!10,colbacklower=white,colframe=black!90,fonttitle=\\bfseries,
title=Example~\\thetcbcounter: #2,#1}
"""

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
        self.requires(core, floats)
        self.addCommand(reader, ExampleCommand())
        self.addCommand(reader, SettingsCommand())

        renderer.add('Example', RenderExample())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('tcolorbox')
            renderer.addPreamble('\\tcbuselibrary{skins}')
            renderer.addPreamble(EXAMPLE_LATEX)
        elif isinstance(renderer, HTMLRenderer):
            renderer.addCSS('devel_moose', "css/devel_moose.css")

class ExampleCommand(command.CommandComponent):
    COMMAND = 'devel'
    SUBCOMMAND = 'example'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        settings['prefix'] = ('Example', settings['prefix'][1])
        return settings

    def createToken(self, parent, info, page, settings):
        flt = floats.create_float(parent, self.extension, self.reader, page, settings)
        ex = Example(flt)

        data = info['block'] if 'block' in info else info['inline']
        code = core.Code(ex, content=data)
        if flt is parent:
            ex.attributes.update(**self.attributes(settings))

        return ex

class SettingsCommand(command.CommandComponent):
    COMMAND = 'devel'
    SUBCOMMAND = 'settings'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['module'] = (None, "The name of the module containing the object.")
        settings['object'] = (None, "The name of the object to import from the 'module'.")
        settings.update(floats.caption_settings())
        settings['prefix'] = ('Table', settings['prefix'][1])

        return settings

    def createToken(self, parent, info, page, settings):
        if settings['module'] is None:
            raise exceptions.MooseDocsException("The 'module' setting is required.")

        if settings['object'] is None:
            raise exceptions.MooseDocsException("The 'object' setting is required.")

        primary = floats.create_float(parent, self.extension, self.reader, page, settings,
                                     token_type=table.TableFloat)
        try:
            mod = importlib.import_module(settings['module'])
        except ImportError:
            msg = "Unable to load the '{}' module."
            raise exceptions.MooseDocsException(msg, settings['module'])

        try:
            obj = getattr(mod, settings['object'])
        except AttributeError:
            msg = "Unable to load the '{}' attribute from the '{}' module."
            raise exceptions.MooseDocsException(msg, settings['object'], settings['module'])

        if hasattr(obj, 'defaultSettings'):
            obj_settings = obj.defaultSettings()
        elif hasattr(obj, 'defaultConfig'):
            obj_settings = obj.defaultConfig()
        else:
            msg = "The '{}' object in the '{}' module does not have a 'defaultSettings' or "\
                  "'defaultConfig' method."
            raise exceptions.MooseDocsException(msg, mod, obj)

        rows = [[key, value[0], value[1]] for key, value in obj_settings.items()]
        tbl = table.builder(rows, headings=['Key', 'Default', 'Description'])
        tbl.parent = primary

        if primary is parent:
            tbl.attributes.update(**self.attributes(settings))

        return primary

class RenderExample(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return parent

    def createMaterialize(self, parent, token, page):

        # Builds the tabs
        div = html.Tag(parent, 'div', class_='moose-devel-example')
        ul = html.Tag(div, 'ul', class_='tabs')
        cid = str(uuid.uuid4())
        html.Tag(html.Tag(ul, 'li', class_='tab'), 'a', href='#{}'.format(cid), string='Markdown')
        oid = str(uuid.uuid4())
        html.Tag(html.Tag(ul, 'li', class_='tab'), 'a', href='#{}'.format(oid), string='HTML')

        # Render the content within the tabs
        div_code = html.Tag(div, 'div', id_=cid, class_='moose-devel-example-code')
        self.translator.renderer.render(html.Tag(div_code, 'pre'),
                                        tokens.String(None, content=token(0)['content']), page)

        div_out = html.Tag(div, 'div', id_=oid, class_='moose-devel-example-html')
        for child in [c for c in token.children[1:]] if len(token) > 1 else list():
            self.translator.renderer.render(div_out, child, page)

        return None

    def createLatex(self, parent, token, page):
        example = latex.Environment(parent, 'example')

        self.translator.renderer.render(example, token(0), page) # upper
        latex.Command(example, 'tcblower', start='\n')
        for child in [c for c in token.children[1:]] if len(token) > 1 else list():
            self.translator.renderer.render(example, child, page)
        return None
