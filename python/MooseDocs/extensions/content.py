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
import mooseutils
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.tree import pages, tokens, html, latex
from MooseDocs.extensions import core, command, heading

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return ContentExtension(**kwargs)

ContentToken = tokens.newToken('ContentToken', location=u'', level=None)
AtoZToken = tokens.newToken('AtoZToken', location=u'', level=None, buttons=bool)

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
        renderer.add('AtoZToken', RenderAtoZ())
        renderer.add('ContentToken', RenderContentToken())

        if isinstance(renderer, LatexRenderer):
            renderer.addPreamble(LATEX_CONTENTLIST)

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

        for value in headings.itervalues():
            value.sort(key=lambda x: x[2])

        return headings

class ContentCommand(command.CommandComponent):
    COMMAND = 'contents' #TODO: Change this to content after format is working
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (None, "The markdown content directory to build contents.")
        settings['level'] = (2, 'Heading level for top-level headings.')
        return settings

    def createToken(self, parent, info, page):
        ContentToken(parent, location=self.settings['location'], level=self.settings['level'])
        return parent

class AtoZCommand(command.CommandComponent):
    COMMAND = 'contents'
    SUBCOMMAND = 'a-to-z'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['level'] = (2, 'Heading level for A, B,... headings.')
        settings['buttons'] = (True, 'Display buttons linking to the A, B,... headings.')
        return settings

    def createToken(self, parent, info, page):
        AtoZToken(parent, level=self.settings['level'], buttons=self.settings['buttons'])
        return parent

class RenderContentToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

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
                    a = html.Tag(h, 'a', href=dest, string=unicode(head) + u' ')
                    html.Tag(a, 'i', string='link', class_='small material-icons moose-inline-icon')
                else:
                    html.String(h, content=unicode(head))

            row = html.Tag(parent, 'div', class_='row')
            for chunk in mooseutils.make_chunks(list(items), 3):
                col = html.Tag(row, 'div', class_='col s12 m6 l4')
                ul = html.Tag(col, 'ul', class_='moose-a-to-z')
                for text, path, _ in chunk:
                    li = html.Tag(ul, 'li')
                    html.Tag(li, 'a', href=path, string=unicode(text.replace('.md', '')))

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
        pass

    def createMaterialize(self, parent, token, page):

        # Initialized alphabetized storage
        headings = self.extension.binContent(page, token['location'], ContentExtension.LETTER)
        for letter in 'abcdefghijklmnopqrstuvwxyz':
            if letter not in headings:
                headings[letter] = set()

        # Buttons
        buttons = html.Tag(parent, 'div', class_='moose-a-to-z-buttons')
        if not token['buttons']:
            buttons.parent = None

        # Build lists
        for letter, items in headings.iteritems():
            id_ = uuid.uuid4()
            btn = html.Tag(buttons, 'a',
                           string=unicode(letter.upper()),
                           class_='btn moose-a-to-z-button',
                           href='#{}'.format(id_))

            if not items:
                btn.addClass('disabled')
                continue

            html.Tag(parent, 'h{:d}'.format(int(token['level'])),
                     class_='moose-a-to-z',
                     id_=unicode(id_),
                     string=unicode(letter))

            row = html.Tag(parent, 'div', class_='row')
            for chunk in mooseutils.make_chunks(list(items), 3):
                col = html.Tag(row, 'div', class_='col s12 m6 l4')
                ul = html.Tag(col, 'ul', class_='moose-a-to-z')
                for text, path, _ in chunk:
                    li = html.Tag(ul, 'li')
                    html.Tag(li, 'a', href=path, string=unicode(text))

    def createLatex(self, parent, token, page):

        headings = self.extension.binContent(page, token['location'], ContentExtension.LETTER)
        latex.Command(parent, 'par', start='\n')
        for items in headings.values():
            for text, path, label in sorted(items, key=lambda x: x[2]):
                args = [latex.Brace(string=path, escape=False),
                        latex.Brace(string=label, escape=False)]
                latex.Command(parent, 'ContentItem', start='\n', args=args, string=text)
            latex.Command(parent, 'par', start='\n')
