#pylint: disable=missing-docstring

import re

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import command, floats
from MooseDocs.tree import html, tokens
from MooseDocs.tree.base import Property

def make_extension(**kwargs):
    return TableExtension(**kwargs)

#TODO: Re-factor into a single Table token
class Table(tokens.Token):
    pass

class TableBody(tokens.Token):
    pass

class TableHead(tokens.Token):
    pass

class TableRow(tokens.Token):
    pass

class TableItem(tokens.Token):
    PROPERTIES = [Property('align', ptype=str, default='center')]

class TableHeaderItem(TableItem):
    pass

def builder(rows, headings=None):
    node = Table()
    if headings:
        thead = TableHead(node)
        row = TableRow(thead)
        for h in headings:
            th = TableHeaderItem(row)
            tokens.String(th, content=unicode(h))

    tbody = TableBody(node)
    for data in rows:
        row = TableRow(tbody)
        for d in data:
            tr = TableItem(row)
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

        self.addCommand(TableCommandComponent())

        reader.addBlock(TableComponent(), "<Paragraph")

        renderer.add(Table, RenderTable())
        renderer.add(TableHead, RenderTag('thead'))
        renderer.add(TableBody, RenderTag('tbody'))
        renderer.add(TableRow, RenderTag('tr'))
        renderer.add(TableHeaderItem, RenderTag('th'))
        renderer.add(TableItem, RenderTag('td'))

class TableCommandComponent(command.CommandComponent):
    COMMAND = 'table'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the listing content.")
        settings['prefix'] = (None, "Text to include prior to the included text.")
        return settings

    def createToken(self, info, parent):

        content = info['block'] if 'block' in info else info['inline']
        flt = floats.Float(parent, **self.attributes)
        floats.add_caption(flt, self.extension, self.settings)
        self.translator.reader.parse(flt, content, group=MooseDocs.BLOCK)
        return parent


class TableComponent(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^(?P<table>\|.*?)(?=\Z|\n{2,})',
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    FORMAT_RE = re.compile(r'^(?P<format>\|[ \|:\-]+\|)$', flags=re.MULTILINE|re.UNICODE)

    def createToken(self, match, parent):

        try:
            return self._createTable(match, parent)
        except Exception as e:
            msg = 'Failed to build table, the syntax is likely not correct:\n'
            raise exceptions.TokenizeException(msg, e.message)

    def _createTable(self, match, parent):

        content = match['table']
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
                #print string, form[i]

        #TODO: check lengths of form, head, body
        if head:
            row = TableRow(TableHead(table))
            for i, h in enumerate(head):
                hitem = TableHeaderItem(row, format=form[i])
                self.reader.parse(hitem, h, MooseDocs.INLINE)

        for line in body.splitlines():
            if line:
                row = TableRow(TableBody(table))
                for i, content in enumerate([item.strip() for item in line.split('|') if item]):
                    item = TableItem(row, format=form[i]) #pylint: disable=redefined-variable-type
                    self.reader.parse(item, content, MooseDocs.INLINE)

        return table

class RenderTable(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', **token.attributes)
        div.addClass('moose-table-div')
        tbl = html.Tag(div, 'table')
        return tbl
    def createMaterialize(self, token, parent):
        return self.createHTML(token, parent)
    def createLatex(self, token, parent):
        pass

class RenderTag(components.RenderComponent):
    def __init__(self, tag):
        components.RenderComponent.__init__(self)
        self.__tag = tag

    def createMaterialize(self, token, parent):
        return self.createHTML(token, parent)

    def createHTML(self, token, parent): #pylint: disable=unused-argument
        return html.Tag(parent, self.__tag)

    def createLatex(self, token, parent):
        pass
