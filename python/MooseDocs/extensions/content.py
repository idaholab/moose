#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import uuid
import collections
import logging
import moosetree
import mooseutils
from .. import common
from ..common import exceptions
from ..base import components, renderers, LatexRenderer
from ..tree import pages, tokens, html, latex
from . import core, command, heading

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return ContentExtension(**kwargs)

ContentToken = tokens.newToken('ContentToken', location='', level=None)
AtoZToken = tokens.newToken('AtoZToken', location='', level=None, buttons=True)
TableOfContents = tokens.newToken('TableOfContents', levels=list(), columns=1, hide=[])
ContentOutline = tokens.newToken('ContentOutline', location=None, recursive=False, pages=list(),
                                 max_level=1, hide=[], no_prefix=[], no_count=[])
PaginationToken = tokens.newToken('PaginationToken', previous=None, next=None, use_title=False,
                                  margins=['24px', '24px'])

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
        self.addCommand(reader, ContentOutlineCommand())
        self.addCommand(reader, PaginationCommand())

        renderer.add('AtoZToken', RenderAtoZ())
        renderer.add('ContentToken', RenderContentToken())
        renderer.add('TableOfContents', RenderTableOfContents())
        renderer.add('ContentOutline', RenderContentOutline())
        renderer.add('PaginationToken', RenderPagination())

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
        for node in nodes:
            h_node = heading.find_heading(node)
            if h_node is not None:
                text = h_node.text()
                # Remove spaces inserted around numbers
                # Numbers are tokens like words, hence the insertion of a separator
                text = re.sub(r'\s(\d)\s', r'\1', text)
                # Replace intended spaces by dashes which are rendered as spaces
                label = text.replace(' ', '-').lower()

                if method == ContentExtension.LETTER:
                    key = label[0]
                elif method == ContentExtension.FOLDER:
                    parts = tuple(node.local.replace(location, '').strip(os.sep).split(os.sep))
                    key = parts[0] if len(parts) > 1 else ''
                else:
                    raise exceptions.MooseDocsException("Unknown method.")
                path = node.relativeDestination(page)
                headings[key].append((text, path, label))

        for value in headings.values():
            value.sort(key=lambda x: x[2])

        return headings

class ContentCommand(command.CommandComponent):
    COMMAND = ('content')
    SUBCOMMAND = (None, 'list')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = ('', "The markdown content directory to build contents.")
        settings['level'] = (2, 'Heading level for top-level headings.')
        return settings

    def createToken(self, parent, info, page, settings):
        ContentToken(parent, location=settings['location'], level=settings['level'])
        return parent

class AtoZCommand(command.CommandComponent):
    COMMAND = ('content')
    SUBCOMMAND = 'a-to-z'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = ('', "The markdown content directory to build contents.")
        settings['level'] = (2, 'Heading level for A, B,... headings.')
        settings['buttons'] = (True, 'Display buttons linking to the A, B,... headings.')
        return settings

    def createToken(self, parent, info, page, settings):
        AtoZToken(parent, location=settings['location'], level=settings['level'],
                  buttons=settings['buttons'])
        return parent

class TableOfContentsCommand(command.CommandComponent):
    COMMAND = ('content')
    SUBCOMMAND = 'toc'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['levels'] = ([1], 'Heading level(s) to display.')
        settings['columns'] = (1, 'The number of columns to display.')
        settings['hide'] = ('', "A list of heading ids to hide.")
        return settings

    def createToken(self, parent, info, page, settings):
        levels = settings['levels']
        if isinstance(levels, (str, str)):
            levels = [int(l) for l in levels.split()]

        return TableOfContents(parent,
                               hide=settings['hide'].split(),
                               levels=levels,
                               columns=int(settings['columns']))

class ContentOutlineCommand(command.CommandComponent):
    COMMAND = 'content'
    SUBCOMMAND = 'outline'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['location'] = (None, "The markdown content directory to build outline.")
        settings['recursive'] = (False, "Include content from the location subdirectories recursively.")
        settings['pages'] = ('', "The pages to include in outline in desired order of appearance.")
        settings['max_level'] = (1, 'Maximum heading level to display.')
        settings['hide'] = ('', "A list of heading ids to hide.")
        settings['no_prefix'] = ('', "A list of heading levels and/or ids to not show the prefixes for.")
        settings['no_count'] = ('', "A list of heading levels and/or ids to not count the indices for.")
        return settings

    def createToken(self, parent, info, page, settings):
        if settings['location'] is None and not settings['pages']:
            msg = "Either the 'location' or the 'pages' setting is required for the !content outline command."
            raise exceptions.MooseDocsException(msg)
        if settings['location'] is not None and settings['pages']:
            msg = "The 'location' and 'pages' settings must be used exclusively."
            raise exceptions.MooseDocsException(msg)
        if settings['recursive'] and settings['pages']:
            msg = "Setting 'recursive=True' has no effect when the 'pages' setting is used."
            LOG.warning(common.report_error(msg, page.source, info.line, info[0], prefix='WARNING'))

        # split strings into lists and convert floats to ints to lists of strings
        no_prefix = settings['no_prefix']
        no_count = settings['no_count']
        if isinstance(no_prefix, (str, str)):
            no_prefix = no_prefix.split()
        else:
            no_prefix = [str(int(no_prefix))]
        if isinstance(no_count, (str, str)):
            no_count = no_count.split()
        else:
            no_count = [str(int(no_count))]

        return ContentOutline(parent,
                              location=settings['location'],
                              recursive=settings['recursive'],
                              pages=settings['pages'].split(),
                              max_level=int(settings['max_level']),
                              hide=settings['hide'].split(),
                              no_prefix=no_prefix,
                              no_count=no_count)

class PaginationCommand(command.CommandComponent):
    COMMAND = 'content'
    SUBCOMMAND = 'pagination'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['previous'] = (None, "The previous markdown page to navigate to.")
        settings['next'] = (None, "The next markdown page to navigate to.")
        settings['use_title'] = (False, "Use the title of the page for the hyperlink text.")
        settings['margin-top'] = ('24px', "The top margin of the buttons.")
        settings['margin-bottom'] = ('24px', "The bottom margin of the buttons.")
        return settings

    def createToken(self, parent, info, page, settings):
        if settings['previous'] is None and settings['next'] is None:
            msg = "At least one: a 'previous' page or a 'next' page is required for the !content pagination command."
            raise exceptions.MooseDocsException(msg)

        margins = [settings['margin-top'], settings['margin-bottom']]
        return PaginationToken(parent,
                               previous=settings['previous'],
                               next=settings['next'],
                               use_title=settings['use_title'],
                               margins=margins)

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
                    html.Tag(h, 'a', href=dest, string=str(head) + ' ')
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
        toks = moosetree.findall(token.root, func)

        div = html.Tag(parent, 'div', class_='moose-table-of-contents')
        div.addStyle('column-count:{}'.format(token['columns']))
        for tok in toks:
            id_ = tok['id']
            bookmark = id_ if id_ else tok.text('-').lower()
            link = core.Link(None, url='#{}'.format(bookmark))
            tok.copyToToken(link)
            core.LineBreak(link)
            self.renderer.render(div, link, page)

    def createLatex(self, parent, token, page):
        return None

class RenderContentOutline(components.RenderComponent):
    def createHTML(self, parent, token, page):
        self.createHTMLHelper(parent, token, page)

    def createMaterialize(self, parent, token, page):
        self.createHTMLHelper(parent, token, page)

    def createHTMLHelper(self, parent, token, page):
        if token['location'] is not None and not token['pages']:
            if token['recursive']:
                func = lambda p: p.local.startswith(token['location']) and isinstance(p, pages.Source)
            else:
                location = token['location'].rstrip('/')
                func = lambda p: os.path.dirname(p.local) == location and isinstance(p, pages.Source)
            nodes = self.translator.findPages(func)
        elif token['pages'] and token['location'] is None:
            nodes = [self.translator.findPage(p) for p in token['pages']]
        else:
            msg = "The 'location' and 'pages' settings must be used exclusively."
            raise exceptions.MooseDocsException(msg)

        # Define convenience variables
        max_level = token['max_level']
        hide = token['hide']
        no_prefix = token['no_prefix']
        no_count = token['no_count']
        if max_level > 6 or max_level < 1:
            raise exceptions.MooseDocsException("The 'max_level' must be set in range of 1 to 6.")

        # Create the outline from the headings of each node.
        # This initializes the html tags to contain this list, where the previous heading level
        # starts at zero thus will always trigger the creation of <ol><li>...</li></ol> within
        # the "li" tag created here.
        previous = 0 # initialize the previous heading level to zero to trigger creation of <ol><li> initially
        li = html.Tag(parent, 'div', class_='moose-outline')
        for node in nodes:
            pageref = str(node.relativeDestination(page))

            for key, head in node['headings'].items():
                current = head['level']
                if current <= max_level and key not in hide:
                    diff = current - previous # change in heading number

                    # At the same level, so add another <li> to the list
                    if diff == 0:
                        li = html.Tag(li.parent, 'li')

                    # Heading level is less, so this heading goes within the current li
                    elif diff > 0:
                        for j in range(diff):
                            li = html.Tag(html.Tag(li, 'ol'), 'li')

                    # Heading level is more, so this heading goes above the current li
                    else:
                        ol = li.parent
                        for i in range(-diff):
                            ol = ol.parent.parent
                        li = html.Tag(ol, 'li')

                    # override classes if list indices are being hidden or not counted
                    if str(current) in no_count or key in no_count:
                        li.addClass('moose-outline-no-count')
                    elif str(current) in no_prefix or key in no_prefix:
                        li.addClass('moose-outline-no-prefix')

                    url = pageref + '#{}'.format(key)
                    link = core.Link(None, url=url)
                    head.copyToToken(link)
                    self.renderer.render(li, link, page)
                    previous = current

    def createLatex(self, parent, token, page):
        msg = "Warning: The Content Extension\'s 'outline' command is not supported for LaTex documents."
        latex.String(parent, content=msg)

class RenderPagination(components.RenderComponent):
    def createHTML(self, parent, token, page):
        style = 'margin-top:{};margin-bottom:{};'.format(*token['margins'])
        div = html.Tag(parent, 'div', class_='moose-content-pagination', style=style)

        if token['previous'] is not None:
            link = self.createHTMLHelper(div, token, page, 'previous')
        if token['next'] is not None:
            link = self.createHTMLHelper(div, token, page, 'next')

    def createMaterialize(self, parent, token, page):
        style = 'margin-top:{};margin-bottom:{};'.format(*token['margins'])
        div = html.Tag(parent, 'div', class_='moose-content-pagination', style=style)

        if token['previous'] is not None:
            link = self.createHTMLHelper(div, token, page, 'previous')
            link.addClass('btn')
            html.Tag(link, 'i', class_='material-icons left', string='arrow_back')
        if token['next'] is not None:
            link = self.createHTMLHelper(div, token, page, 'next')
            link.addClass('btn')
            html.Tag(link, 'i', class_='material-icons right', string='arrow_forward')

    def createHTMLHelper(self, parent, token, page, direction):
        node = self.translator.findPage(token[direction])

        # Determine hyperlink style.
        if token['use_title']:
            string = heading.find_heading(node).text()
            if len(string) > 18:
                string = string[:18] + '. . .'
        else:
            string = direction.capitalize()

        return html.Tag(parent, 'a',
                        class_='moose-content-{}'.format(direction),
                        href=str(node.relativeDestination(page)),
                        string=string)

    def createLatex(self, parent, token, page):
        msg = "Warning: The Content Extension\'s 'pagination' command is not supported for LaTeX documents."
        latex.String(parent, content=msg)
