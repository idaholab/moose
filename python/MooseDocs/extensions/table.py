#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import html, tokens, latex
from . import command, floats

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
                tokens.String(th, content=str(h))

    tbody = TableBody(node)
    for data in rows:
        row = TableRow(tbody)
        for d in data:
            tr = TableItem(row, align='left')
            if isinstance(d, tokens.Token):
                d.parent = tr
            else:
                tokens.String(tr, content=str(d))

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
        config['prefix'] = ('Table', "The caption prefix (e.g., Tbl.).")
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

    def createToken(self, parent, info, page, settings):
        content = info['block'] if 'block' in info else info['inline']
        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  token_type=TableFloat)
        self.reader.tokenize(flt, content, page, MarkdownReader.BLOCK, line=info.line)

        if flt is parent:
            parent(0).attributes.update(**self.attributes(settings))

        return parent

class TableComponent(components.ReaderComponent):
    RE = re.compile(r'(?:\A|\n{2,})^(?P<table>\|.*?)(?=\n*\Z|\n{2,})',
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    FORMAT_RE = re.compile(r'^(?P<format>\|[ \|:\-]+\|)$', flags=re.MULTILINE|re.UNICODE)
    SPLIT_RE = re.compile(r'(?:\A| )\|(?: |\Z)')

    def createToken(self, parent, info, page, settings):
        content = info['table']
        table = Table(parent)
        head = None
        body = None
        form = None

        format_match = self.FORMAT_RE.search(content)
        if format_match:
            head = [item.strip() for item in \
                    content[:format_match.start('format')-1].split('|') if item]
            body = content[format_match.end('format'):]
            form = [item.strip() for item in format_match.group('format').split('|') if item]

        if form:
            for i, string in enumerate(form):
                if string.startswith(':'):
                    form[i] = 'left'
                elif string.endswith(':'):
                    form[i] = 'right'
                elif string.startswith('-'):
                    form[i] = 'center'
                else:
                    # TODO: warning/error
                    form[i] = 'left'

        #TODO: check lengths of form, head, body
        if head:
            row = TableRow(TableHead(table))
            for i, h in enumerate(head):
                hitem = TableHeadItem(row, align=form[i])
                self.reader.tokenize(hitem, h, page, MarkdownReader.INLINE)

        for line in body.splitlines():
            if line:
                row = TableRow(TableBody(table))
                items = [item.strip() for item in self.SPLIT_RE.split(line) if item]
                for i, content in enumerate(items):
                    item = TableItem(row, align=form[i])
                    self.reader.tokenize(item, content, page, MarkdownReader.INLINE)

        table['form'] = form
        return table

class RenderTable(components.RenderComponent):
    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div', **token.attributes)
        div.addClass('moose-table-div')
        tbl = html.Tag(div, 'table')
        return tbl

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):

        args = [latex.Brace(string='\\textwidth', escape=False),
                latex.Brace(string="".join([f[0].upper() for f in token['form']]))]
        return latex.Environment(parent, 'tabulary', start='\\par', args=args)

class RenderTag(components.RenderComponent):
    def __init__(self, tag):
        components.RenderComponent.__init__(self)
        self.__tag = tag

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createHTML(self, parent, token, page):
        return html.Tag(parent, self.__tag)

    def createLatex(self, parent, token, page):

        items = parent
        if token.name == 'TableHead':
            latex.String(parent, content='\\toprule\n', escape=False)
            items = latex.String(parent)
            latex.String(parent, content='\\midrule\n', escape=False)
        elif (token.name == 'TableBody') and (token is token.parent.children[-1]):
            items = latex.String(parent)
            latex.String(parent, content='\\bottomrule', escape=False)
        return items

class RenderItem(RenderTag):
    def createHTML(self, parent, token, page):
        tag = RenderTag.createHTML(self, parent, token, page)
        tag.addStyle('text-align:{}'.format(token['align']))
        return tag

    def createLatex(self, parent, token, page):

        item = latex.String(parent)
        end = ' \\\\\n' if token is token.parent.children[-1] else ' &'
        latex.String(parent, content=end, escape=False)

        return item

class RenderTableFloat(floats.RenderFloat):

    def createLatex(self, parent, token, page):
        #token.children = reversed(token.children)
        token['command'] = 'table'
        flt = floats.RenderFloat.createLatex(self, parent, token, page)
        latex.Command(flt, 'center')
        return flt
