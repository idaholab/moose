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
from MooseDocs.base import components
from MooseDocs.extensions import command, floats
from MooseDocs.tree import html, tokens

def make_extension(**kwargs):
    return TableExtension(**kwargs)

Table = tokens.newToken('Table')
TableBody = tokens.newToken('TableBody')
TableHead = tokens.newToken('TableHead')
TableHeadItem = tokens.newToken('TableHeadItem')
TableRow = tokens.newToken('TableRow')
TableItem = tokens.newToken('TableItem', align='center')

def builder(rows, headings=None):
    """Helper for creating tokens for a table."""
    node = Table(None)
    if headings:
        thead = TableHead(node)
        row = TableRow(thead)
        for h in headings:
            th = TableHeadItem(row, align='left')
            tokens.String(th, content=unicode(h))

    tbody = TableBody(node)
    for data in rows:
        row = TableRow(tbody)
        for d in data:
            tr = TableItem(row, align='left')
            tokens.String(tr, content=unicode(d))

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

class TableCommandComponent(command.CommandComponent):
    COMMAND = 'table'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['prefix'] = (None, "Text to include prior to the included text.")
        return settings

    def createToken(self, parent, info, page):

        content = info['block'] if 'block' in info else info['inline']
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings)
        self.reader.tokenize(flt, content, page, MooseDocs.BLOCK)
        return parent

class TableComponent(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^(?P<table>\|.*?)(?=\Z|\n{2,})',
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    FORMAT_RE = re.compile(r'^(?P<format>\|[ \|:\-]+\|)$', flags=re.MULTILINE|re.UNICODE)

    def createToken(self, parent, info, page):

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
                self.reader.tokenize(hitem, h, page, MooseDocs.INLINE)

        for line in body.splitlines():
            if line:
                row = TableRow(TableBody(table))
                for i, content in enumerate([item.strip() for item in line.split('|') if item]):
                    item = TableItem(row, align=form[i]) #pylint: disable=redefined-variable-type
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
        pass

class RenderTag(components.RenderComponent):
    def __init__(self, tag):
        components.RenderComponent.__init__(self)
        self.__tag = tag

    def createMaterialize(self, parent, token, page):
        return self.createHTML(parent, token, page)

    def createHTML(self, parent, token, page): #pylint: disable=unused-argument
        return html.Tag(parent, self.__tag)

    def createLatex(self, parent, token, page):
        pass

class RenderItem(RenderTag):
    def createHTML(self, parent, token, page):
        tag = RenderTag.createHTML(self, parent, token, page)
        tag.addStyle('text-align:{}'.format(token['align']))
        return tag
