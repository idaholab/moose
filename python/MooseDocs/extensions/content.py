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
import mooseutils
from MooseDocs.base import components
from MooseDocs.tree import pages, tokens, html
from MooseDocs.extensions import core, command, heading

def make_extension(**kwargs):
    return ContentExtension(**kwargs)

Collapsible = tokens.newToken('Collapsible', summary=u'')
ContentToken = tokens.newToken('ContentToken', location=u'')
AtoZToken = tokens.newToken('AtoZToken', location=u'', level=None, buttons=bool)

class ContentExtension(command.CommandExtension):
    """
    Allows for the creation of markdown contents lists.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(core, heading, command)
        self.addCommand(reader, ContentCommand())
        self.addCommand(reader, AtoZCommand())
        renderer.add('Collapsible', RenderCollapsible())
        renderer.add('AtoZToken', RenderAtoZ())

class ContentCommand(command.CommandComponent):
    COMMAND = 'contents' #TODO: Change this to content after format is working
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (None, "The markdown content directory to build contents.")
        return settings

    def createToken(self, parent, info, page):

        tree = dict()
        tree[(u'',)] = core.UnorderedList(parent, browser_default=False)

        location = self.settings['location']

        func = lambda p: p.local.startswith(location) and isinstance(p, pages.Source)
        nodes = self.translator.findPages(func)
        for node in nodes:
            key = tuple(node.local.strip(os.sep).replace(location, '').split(os.sep))[:-1]
            for i in xrange(1, len(key) + 1):
                k = key[:i]
                if k not in tree:
                    col = Collapsible(tree[k[:-1]], summary=k[-1])
                    li = core.ListItem(col, class_='moose-source-item', tooltip=False)
                    tree[k] = core.UnorderedList(li, browser_default=False)

        for node in nodes:
            key = tuple(node.local.strip(os.sep).replace(location, '').split(os.sep))[:-1]
            loc = node.relativeDestination(page)
            li = core.ListItem(tree[key])
            core.Link(li, url=loc, string=node.name, class_='moose-source-item', tooltip=False)

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
        return AtoZToken(parent, level=self.settings['level'], buttons=self.settings['buttons'])

class RenderCollapsible(components.RenderComponent):
    def createHTML(self, parent, token, page):

        details = html.Tag(parent, 'details')
        summary = html.Tag(details, 'summary')
        html.Tag(summary, 'span', class_='moose-section-icon')
        html.Tag(summary, 'span', string=token['summary'])
        return details

    def createLatex(self, parent, token, page):
        pass


class RenderAtoZ(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createMaterialize(self, parent, token, page):

        # Initialized alphabetized storage
        headings = dict()
        for letter in 'ABCDEFGHIJKLNMOPQRSTUVWXYZ':
            headings[letter] = dict()

        # Extract headings, default to filename if a heading is not found
        func = lambda n: n.local.startswith(token['location']) and isinstance(n, pages.Source)
        for node in self.translator.findPages(func):
            h_node = heading.find_heading(self.translator, node)
            if h_node is not None:
                r = html.Tag(None, 'span')
                self.renderer.render(r, h_node, page)
                key = r.text()
            else:
                r = None
                key = node.name

            letter = key[0].upper()
            headings[letter][key] = node.relativeDestination(page)

        # Buttons
        buttons = html.Tag(parent, 'div', class_='moose-a-to-z-buttons')
        if not token['buttons']:
            buttons.parent = None

        # Build lists
        for letter, items in headings.iteritems():
            id_ = uuid.uuid4()
            btn = html.Tag(buttons, 'a',
                           string=unicode(letter),
                           class_='btn moose-a-to-z-button',
                           href='#{}'.format(id_))

            if not items:
                btn.addClass('disabled')
                continue

            html.Tag(parent, 'h{}'.format(token['level']),
                     class_='moose-a-to-z',
                     id_=unicode(id_),
                     string=unicode(letter))

            row = html.Tag(parent, 'div', class_='row')

            links = [(text, href) for text, href in items.iteritems()]
            for chunk in mooseutils.make_chunks(links, 3):
                col = html.Tag(row, 'div', class_='col s12 m6 l4')
                ul = html.Tag(col, 'ul', class_='moose-a-to-z')
                for text, href in chunk:
                    li = html.Tag(ul, 'li')
                    html.Tag(li, 'a', href=href, string=unicode(text))

    def createLatex(self, parent, token, page):
        pass
