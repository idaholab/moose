#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re

import MooseDocs
from MooseDocs.base import components, LatexRenderer
from MooseDocs import common
from MooseDocs.extensions import command, floats
from MooseDocs.tree import html, tokens, latex

def make_extension(**kwargs):
    return TableExtension(**kwargs)

Table = tokens.newToken('Table', form=[])
TableBody = tokens.newToken('TableBody')
TableHead = tokens.newToken('TableHead')
TableHeadItem = tokens.newToken('TableHeadItem')
TableRow = tokens.newToken('TableRow')
TableItem = tokens.newToken('TableItem', align='center')
TableFloat = tokens.newToken('TableFloat', floats.Float)

def builder(rows, headings=None, form=None):
    """Helper for creating tokens for a table."""
    node = Table(None)
    if headings:
        thead = TableHead(node)
        row = TableRow(thead)
        for h in headings:
            th = TableHeadItem(row, align='left')
            if isinstance(h, tokens.Token):
                h.parent = th
            else:
                tokens.String(th, content=unicode(h))

    tbody = TableBody(node)
    for data in rows:
        row = TableRow(tbody)
        for d in data:
            tr = TableItem(row, align='left')
            if isinstance(d, tokens.Token):
                d.parent = tr
            else:
                tokens.String(tr, content=unicode(d))

    if form is None:
        form = 'L'*len(rows[0])

    node['form'] = form
    return node

class TableExtension(command.CommandExtension):
    """
    Adds markdown style tables and !table command for creating numbered tables.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = (u'Table', "The caption prefix (e.g., Tbl.).")
        return config

    def extend(self, reader, renderer):

        self.addCommand(reader, TableCommandComponent())

        reader.addBlock(TableComponent(), "<ParagraphBlock")

        renderer.add('Table', RenderTable())
        renderer.add('TableHead', RenderTag('thead'))
        renderer.add('TableBody', RenderTag('tbody'))
        renderer.add('TableRow', RenderTag('tr'))
        renderer.add('TableHeadItem', RenderItem('th'))
        renderer.add('TableItem', RenderItem('td'))
        renderer.add('TableFloat', RenderTableFloat())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('tabulary')
            renderer.addPackage('booktabs')

class TableCommandComponent(command.CommandComponent):
    COMMAND = 'table'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page):

        content = info['block'] if 'block' in info else info['inline']
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  token_type=TableFloat)
        self.reader.tokenize(flt, content, page, MooseDocs.BLOCK)

        if flt is parent:
            parent(0).attributes.update(**self.attributes)

        return parent

class TableComponent(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^(?P<table>\|.*?)(?=\Z|\n{2,})',
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    TABLE_FORMAT_DEFAULTS = {'color' : 'black', 'text-align' : 'left'}
    # CSS styles we want to support, but otherwise do not want to set a default for
    SUPPORTED_FORMATS = ['width', 'background-color', 'vertical-align', 'border']

    def __replaceToken(self, md_token, css, styles):
        if re.search(r'(^|\s)' + md_token + r'($|\s)', styles):
            return styles.replace(md_token, css, 1)
        return styles

    def createToken(self, parent, info, page):
        content = info['table'].split('\n')
        table = Table(parent)
        table_format = dict()
        head = None
        form = None

        # Table with formating rules
        if len(content) > 2:
            head = content[0].split('|')
            form = content[1].split('|')
            body = content[2:]

        # A basic table without headers
        else:
            body = content

        if head:
            for i, styles in enumerate([x for x in form if x]):
                # Support markdown alignment syntax
                for md_token, css in [(':-', 'text-align=left'),
                                      ('-:', 'text-align=right'),
                                      ('-', 'text-align=center')]:
                    styles = self.__replaceToken(md_token, css, styles)

                table_format[i], unknown = common.match_settings(self.TABLE_FORMAT_DEFAULTS, styles)
                for style_key, value in unknown.iteritems():
                    if style_key in self.SUPPORTED_FORMATS:
                        table_format[i][style_key] = value

            row = TableRow(TableHead(table))
            for i, h in enumerate([x for x in head if x]):
                hitem = TableHeadItem(row, **table_format[i])
                self.reader.tokenize(hitem, h, page, MooseDocs.INLINE)

        if body:
            for line in body:
                if line:
                    row = TableRow(TableBody(table))
                    for i, content in enumerate([item.strip() for item in line.split('|') if item]):
                        if table_format:
                            styles = table_format[i]
                        else:
                            styles, unknown = common.match_settings(self.TABLE_FORMAT_DEFAULTS, '')

                        item = TableItem(row, **styles)
                        self.reader.tokenize(item, content, page, MooseDocs.INLINE)
        return table

class RenderTable(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', **token.attributes)
        div.addClass('moose-table-div')
        tbl = html.Tag(div, 'table')
        return tbl

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):

        args = [latex.Brace(string=u'\\textwidth', escape=False),
                latex.Brace(string=u"".join([f[0].upper() for f in token['form']]))]
        return latex.Environment(parent, 'tabulary', start='\\par', args=args)

class RenderTag(components.RenderComponent):
    def __init__(self, tag):
        components.RenderComponent.__init__(self)
        self.__tag = tag

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createHTML(self, parent, token, page): #pylint: disable=unused-argument
        return html.Tag(parent, self.__tag)

    def createLatex(self, parent, token, page):

        items = parent
        if token.name == 'TableHead':
            latex.String(parent, content=u'\\toprule\n', escape=False)
            items = latex.String(parent)
            latex.String(parent, content=u'\\midrule\n', escape=False)
        elif (token.name == 'TableBody') and (token is token.parent.children[-1]):
            items = latex.String(parent)
            latex.String(parent, content=u'\\bottomrule', escape=False)
        return items

class RenderItem(RenderTag):
    def createHTML(self, parent, token, page):
        tag = RenderTag.createHTML(self, parent, token, page)
        #tag.addStyle('text-align:{}'.format(token['align']))
        for style_key, style_value in token.iteritems():
            tag.addStyle('%s:%s' % (style_key, style_value))
        return tag

    def createLatex(self, parent, token, page):

        item = latex.String(parent)
        end = u' \\\\\n' if token is token.parent.children[-1] else u' &'
        latex.String(parent, content=end, escape=False)

        return item

class RenderTableFloat(floats.RenderFloat):

    def createLatex(self, parent, token, page):
        #token.children = reversed(token.children)
        token['command'] = 'table'
        flt = floats.RenderFloat.createLatex(self, parent, token, page)
        latex.Command(flt, 'center')
        return flt
