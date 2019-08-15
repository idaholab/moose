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
import uuid
import collections
import logging
import anytree
import mooseutils
from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.base import components, renderers, LatexRenderer
from MooseDocs.tree import pages, tokens, html, latex
from MooseDocs.extensions import core, command, heading

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return ContentExtension(**kwargs)

ContentToken = tokens.newToken('ContentToken', location=u'', level=None)
AtoZToken = tokens.newToken('AtoZToken', location=u'', level=None, buttons=True)
TableOfContents = tokens.newToken('TableOfContents', levels=list(), columns=1, hide=[])

LATEX_CONTENTLIST = """
\\DeclareDocumentCommand{\\ContentItem}{mmm}{#3 (\\texttt{\\small #1})\\dotfill \\pageref{#2}\\\\}
"""

class ContentExtension(command.CommandExtension):
    """
    Allows for the creation of markdown contents lists.
    """
    LETTER = 10
    FOLDER = 11

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['source_links'] = (dict(), "Dictionary of folder name to markdown links.")
        return config

    def extend(self, reader, renderer):
        self.requires(core, heading, command)
        self.addCommand(reader, ContentCommand())
        self.addCommand(reader, AtoZCommand())
        self.addCommand(reader, TableOfContentsCommand())

        renderer.add('AtoZToken', RenderAtoZ())
        renderer.add('ContentToken', RenderContentToken())
        renderer.add('TableOfContents', RenderTableOfContents())

        if isinstance(renderer, LatexRenderer):
            renderer.addPreamble(LATEX_CONTENTLIST)

        if isinstance(renderer, renderers.HTMLRenderer):
            renderer.addCSS('content_moose', "css/content_moose.css")

    def binContent(self, page, location=None, method=None):
        """
        Helper method for creating page bins.

        Inputs:
            location[str]: The content page local path must begin with the given string.
            method[LETTER|FOLDER]: Method for bin assignment.
        """

        location = location
        func = lambda p: p.local.startswith(location) and isinstance(p, pages.Source)
        nodes = self.translator.findPages(func)
        nodes.sort(key=lambda n: n.local)

        headings = collections.defaultdict(list)
        func = lambda n: n.local.startswith(location) and isinstance(n, pages.Source)
        for node in nodes:
            h_node = heading.find_heading(self.translator, node)

            if h_node is None:
                pass
                #msg = "The page, '%s', does not have a title, it will be ignored in the " \
                #      "content output."
                #LOG.warning(msg, node.local)

            else:
                text = h_node.text()
                label = text.replace(' ', '-').lower()
                if method == ContentExtension.LETTER:
                    key = label[0]
                elif method == ContentExtension.FOLDER:
                    parts = tuple(node.local.replace(location, '').strip(os.sep).split(os.sep))
                    key = parts[0] if len(parts) > 1 else u''
                else:
                    raise exceptions.MooseDocsException("Unknown method.")
                path = node.relativeDestination(page)
                headings[key].append((text, path, label))

        for value in headings.values():
            value.sort(key=lambda x: x[2])

        return headings

class ContentCommand(command.CommandComponent):
    COMMAND = ('content', 'contents') #TODO: Change this to content after format is working
    SUBCOMMAND = (None, 'list')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (u'', "The markdown content directory to build contents.")
        settings['level'] = (2, 'Heading level for top-level headings.')
        return settings

    def createToken(self, parent, info, page):
        if info['command'] == 'contents':
            msg = 'The command "!contents" is deprecated, please use "!content list".'
            LOG.warning(common.report_error(msg, page.source, info.line, info[0], prefix='WARNING'))
        ContentToken(parent, location=self.settings['location'], level=self.settings['level'])
        return parent

class AtoZCommand(command.CommandComponent):
    COMMAND = ('content', 'contents')
    SUBCOMMAND = 'a-to-z'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (u'', "The markdown content directory to build contents.")
        settings['level'] = (2, 'Heading level for A, B,... headings.')
        settings['buttons'] = (True, 'Display buttons linking to the A, B,... headings.')
        return settings

    def createToken(self, parent, info, page):
        if info['command'] == 'contents':
            msg = 'The command "!contents a-to-z" is deprecated, please use "!content a-to-z".'
            LOG.warning(common.report_error(msg, page.source, info.line, info[0], prefix='WARNING'))
        AtoZToken(parent, location=self.settings['location'], level=self.settings['level'],
                  buttons=self.settings['buttons'])
        return parent

class TableOfContentsCommand(command.CommandComponent):
    COMMAND = ('content', 'contents')
    SUBCOMMAND = 'toc'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['levels'] = (1, 'Heading level(s) to display.')
        settings['columns'] = (1, 'The number of columns to display.')
        settings['hide'] = ('', "A list of heading ids to hide.")
        return settings

    def createToken(self, parent, info, page):
        if info['command'] == 'contents':
            msg = 'The command "!contents toc" is deprecated, please use "!content toc".'
            LOG.warning(common.report_error(msg, page.source, info.line, info[0], prefix='WARNING'))

        levels = self.settings['levels']
        if isinstance(levels, (str, str)):
            levels = [int(l) for l in levels.split()]

        return TableOfContents(parent,
                               hide=self.settings['hide'].split(),
                               levels=levels,
                               columns=int(self.settings['columns']))

class RenderContentToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        headings = self.extension.binContent(page, token['location'], ContentExtension.FOLDER)
        links = self.extension.get('source_links')

        # Build lists
        for head in sorted(headings.keys()):
            items = headings[head]
            if head:
                h = html.Tag(parent, 'h{:d}'.format(int(token['level'])),
                             class_='moose-a-to-z')
                if head in links:
                    p = self.translator.findPage(links[head])
                    dest = p.relativeDestination(page)
                    html.Tag(h, 'a', href=dest, string=str(head) + u' ')
                else:
                    html.String(h, content=str(head))

            row = html.Tag(parent, 'div', class_='row')
            for chunk in mooseutils.make_chunks(list(items), 3):
                col = html.Tag(row, 'div', class_='col s12 m6 l4')
                ul = html.Tag(col, 'ul', class_='moose-a-to-z')
                for text, path, _ in chunk:
                    li = html.Tag(ul, 'li')
                    html.Tag(li, 'a', href=path, string=str(text.replace('.md', '')))

    def createLatex(self, parent, token, page):

        headings = self.extension.binContent(page, token['location'], ContentExtension.FOLDER)
        latex.Command(parent, 'par', start='\n')
        for key in sorted(headings.keys()):
            items = headings[key]
            for text, path, label in sorted(items, key=lambda x: x[2]):
                args = [latex.Brace(string=path, escape=False),
                        latex.Brace(string=label, escape=False)]
                latex.Command(parent, 'ContentItem', start='\n', args=args, string=text)
            latex.Command(parent, 'par', start='\n')

class RenderAtoZ(components.RenderComponent):

    def createHTML(self, parent, token, page):
        token['buttons'] = False
        self.createHTMLHelper(parent, token, page)

    def createMaterialize(self, parent, token, page):
        self.createHTMLHelper(parent, token, page)

    def createHTMLHelper(self, parent, token, page):

        # Initialized alphabetized storage
        headings = self.extension.binContent(page, token['location'], ContentExtension.LETTER)
        for letter in '0123456789abcdefghijklmnopqrstuvwxyz':
            if letter not in headings:
                headings[letter] = set()

        # Buttons
        buttons = html.Tag(parent, 'div', class_='moose-a-to-z-buttons')
        if not token['buttons']:
            buttons.parent = None

        # Build lists
        for letter in sorted(headings.keys()):
            items = headings[letter]
            id_ = uuid.uuid4()
            btn = html.Tag(buttons, 'a',
                           string=str(letter.upper()),
                           class_='btn moose-a-to-z-button',
                           href='#{}'.format(id_))

            if not items:
                btn.addClass('disabled')
                continue

            html.Tag(parent, 'h{:d}'.format(int(token['level'])),
                     class_='moose-a-to-z',
                     id_=str(id_),
                     string=str(letter))

            row = html.Tag(parent, 'div', class_='row')
            for chunk in mooseutils.make_chunks(list(items), 3):
                col = html.Tag(row, 'div', class_='col s12 m6 l4')
                ul = html.Tag(col, 'ul', class_='moose-a-to-z')
                for text, path, _ in chunk:
                    li = html.Tag(ul, 'li')
                    html.Tag(li, 'a', href=path, string=str(text))

    def createLatex(self, parent, token, page):

        headings = self.extension.binContent(page, token['location'], ContentExtension.LETTER)
        latex.Command(parent, 'par', start='\n')
        for items in headings.values():
            for text, path, label in sorted(items, key=lambda x: x[2]):
                args = [latex.Brace(string=path, escape=False),
                        latex.Brace(string=label, escape=False)]
                latex.Command(parent, 'ContentItem', start='\n', args=args, string=text)
            latex.Command(parent, 'par', start='\n')

class RenderTableOfContents(components.RenderComponent):

    def createHTML(self, parent, token, page):
        hide = token['hide']
        levels = token['levels']
        func = lambda n: (n.name == 'Heading') and (n['level'] in levels) and (n is not token) \
               and (n['id'] not in hide)
        toks = anytree.search.findall(token.root, filter_=func)

        div = html.Tag(parent, 'div', class_='moose-table-of-contents')
        div.addStyle('column-count:{}'.format(token['columns']))
        for tok in toks:
            id_ = tok['id']
            bookmark = id_ if id_ else tok.text(u'-').lower()
            link = core.Link(None, url='#{}'.format(bookmark))
            tok.copyToToken(link)
            core.LineBreak(link)
            self.renderer.render(div, link, page)

    def createLatex(self, parent, token, page):
        return None
