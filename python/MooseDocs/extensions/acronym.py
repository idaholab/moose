#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
import logging
import moosetree
from ..base import components, renderers
from ..common import exceptions
from ..tree import pages, tokens, html, latex
from . import command, table, floats

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return AcronymExtension(**kwargs)

AcronymItem = collections.namedtuple('AcronymItem', 'key name used')
AcronymToken = tokens.newToken('AcronymToken', acronym='')
AcronymListToken = tokens.newToken('AcronymListToken', complete=False, location=None, heading=True)

class AcronymExtension(command.CommandExtension):
    """
    Adds ability to create acronyms within the configuration and use them throughout the pages.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['acronyms'] = (dict(), "Complete dict (or dict of dict) of acronyms.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self.__acronyms = dict()
        self.__used = set() # this is so that in-line definitions are rendered only once per page

        # Initialize the available acronyms
        for key, value in self.get('acronyms').items():
            if isinstance(value, dict):
                self.__acronyms.update(value)
            else:
                self.__acronyms[key] = value

    def initPage(self, page):
        page['acronyms'] = dict()

    def preExecute(self):
        """
        Reinitialize the list of acronyms being used.
        """
        self.__used = set()

    def postTokenize(self, page, ast):
        """
        Adds a list of valid acronyms to the page attributes.
        """
        func = lambda n: (n.name == 'AcronymToken')
        for node in moosetree.iterate(ast.root, func):
            acro = node.get('acronym')
            if acro in self.__acronyms.keys() and acro not in page['acronyms'].keys():
                page['acronyms'][acro] = self.__acronyms.get(acro)

    def getAcronym(self, key):
        """
        Return an AcronymItem given the supplied key. Raise error if it is not found.
        """
        acro = self.__acronyms.get(key, None)
        if acro is not None:
            used = key in self.__used
            if not used:
                self.__used.add(key)
            acro = AcronymItem(key=key, name=acro, used=used)

        return acro

    def getAcronyms(self, page, complete=False):
        """
        Return the used or complete set of acronyms.
        """
        if complete:
            return self.__acronyms
        else:
            return page['acronyms']

    def extend(self, reader, renderer):
        self.requires(command, table, floats)
        self.addCommand(reader, AcronymComponent())
        self.addCommand(reader, AcronymListComponent())
        renderer.add('AcronymToken', RenderAcronymToken())
        renderer.add('AcronymListToken', RenderAcronymListToken())

        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage('tabulary')

class AcronymComponent(command.CommandComponent):
    COMMAND = 'ac'
    SUBCOMMAND = None

    def createToken(self, parent, info, page, settings):
        AcronymToken(parent, acronym=info['inline'])
        return parent

class AcronymListComponent(command.CommandComponent):
    COMMAND = 'acronym'
    SUBCOMMAND = 'list'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['complete'] = (False, "Show the complete list of acronyms regardless of use on " \
                                       "current page.")
        settings['location'] = (None, "The markdown content directory to build the list from.")
        settings['heading'] = (True, "Display the headings row of the acronym table.")
        settings['prefix'] = ('Table', "Prefix to use when a caption and id are provided.")
        settings['caption'] = (None, "The caption to use for the acronym table.")
        return settings

    def createToken(self, parent, info, page, settings):
        if settings['location'] and settings['complete']:
            msg = "The 'complete' setting must be 'False' (default) when using 'location'."
            raise exceptions.MooseDocsException(msg)

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  **self.attributes(settings))
        acro = AcronymListToken(flt,
                                complete=settings['complete'],
                                location=settings['location'],
                                heading=settings['heading'])
        if flt is parent:
            acro.attributes.update(**self.attributes(settings))

        return parent

class RenderAcronymToken(components.RenderComponent):

    def _createSpan(self, parent, token, page):
        acro = self.extension.getAcronym(token['acronym'])
        if acro is None:
            tag = html.Tag(parent, 'span', string=token['acronym'], class_='moose-error')
            raise exceptions.MooseDocsException("The acronym '{}' was not found.", token['acronym'])

        else:
            content = str(acro.key) if acro.used else '{} ({})'.format(acro.name, acro.key)
            tag = html.Tag(parent, 'span', string=content)

        return tag, acro

    def createHTML(self, parent, token, page):
        self._createSpan(parent, token, page)

    def createMaterialize(self, parent, token, page):
        span, acro = self._createSpan(parent, token, page)
        if (acro is not None) and acro.used:
            span.addClass('tooltipped')
            span['data-tooltip'] = acro.name
            span['data-position'] = 'top'
            span['data-delay'] = 50

    def createLatex(self, parent, token, page):
        acro = self.extension.getAcronym(token['acronym'])
        content = str(acro.key) if acro.used else '{} ({})'.format(acro.name, acro.key)
        latex.String(parent, content=content)

class RenderAcronymListToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        rows = []
        if token['location'] is None:
            for key, value in self.extension.getAcronyms(page, token['complete']).items():
                rows.append([key, value])

        elif not token['complete']:
            listed = [] # keeps track of which acronyms have already been listed
            func = lambda p: p.local.startswith(token['location']) and isinstance(p, pages.Source)
            for node in self.translator.findPages(func):
                for key, value in self.extension.getAcronyms(node, False).items():
                    if key not in listed:
                        rows.append([key, value])
                        listed.append(key)

        else:
            msg = "The 'complete' setting must be 'False' (default) when using 'location'."
            raise exceptions.MooseDocsException(msg)

        if rows:
            heading = ['Acronym', 'Description'] if token['heading'] else None
            rows.sort() # alphabetize the acronym list
            tbl = table.builder(rows, heading)
            self.renderer.render(parent, tbl, page)

    def createLatex(self, parent, token, page):

        env = latex.Environment(parent, 'tabulary',
                                args=[latex.Brace(None, string='\\linewidth', escape=False),
                                      latex.Brace(None, string='LL')])

        if token['location'] is None:
            for key, value in self.extension.getAcronyms(page, token['complete']).items():
                latex.String(env, content='{}&{}\\\\'.format(key, value), escape=False)

        elif not token['complete']:
            listed = [] # keeps track of which acronyms have already been listed
            func = lambda p: p.local.startswith(token['location']) and isinstance(p, pages.Source)
            for node in self.translator.findPages(func):
                for key, value in self.extension.getAcronyms(node, False).items():
                    if key not in listed:
                        latex.String(env, content='{}&{}\\\\'.format(key, value), escape=False)
                        listed.append(key)

        else:
            msg = "Warning: The 'complete' setting is invalid when using 'location'."
            latex.String(parent, content=msg)
