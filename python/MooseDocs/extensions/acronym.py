#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections

from MooseDocs.base import components, renderers
from MooseDocs.common import exceptions
from MooseDocs.extensions import command, table, floats
from MooseDocs.tree import tokens, html, latex

def make_extension(**kwargs):
    return AcronymExtension(**kwargs)

AcronymItem = collections.namedtuple('AcronymItem', 'key name used')
AcronymToken = tokens.newToken('AcronymToken', acronym=u'')
AcronymListToken = tokens.newToken('AcronymListToken', heading=True)

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
        self.__used = set()

        # Initialize the available acronyms
        for key, value in self.get('acronyms').items(): #pylint: disable=no-member
            if isinstance(value, dict):
                self.__acronyms.update(value)
            else:
                self.__acronyms[key] = value

    def preExecute(self, root):
        """
        Reinitialize the list of acronyms being used.
        """
        self.__used = set()

    def getAcronym(self, key):
        """
        Return an AcronymItem given the supplied key, None is returned if it is not found.
        """
        acro = self.__acronyms.get(key, None)
        if acro is not None:
            used = key in self.__used
            if not used:
                self.__used.add(key)
            acro = AcronymItem(key=key, name=acro, used=used)

        else:
            msg = "The acronym '{}' was not found."
            raise exceptions.MooseDocsException(msg, key)

        return acro

    def getAcronyms(self, complete=False):
        """
        Return the used or complete set of acronyms.
        """
        if complete:
            return self.__acronyms
        return {k:self.__acronyms[k] for k in self.__used}

    def extend(self, reader, renderer):
        self.requires(command, table, floats)
        self.addCommand(reader, AcronymComponentOld())
        self.addCommand(reader, AcronymComponent())
        self.addCommand(reader, AcronymListComponent())
        renderer.add('AcronymToken', RenderAcronymToken())
        renderer.add('AcronymListToken', RenderAcronymListToken())

        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage('tabulary')

class AcronymComponentOld(command.CommandComponent):
    COMMAND = 'ac'
    SUBCOMMAND = '*'

    def createToken(self, parent, info, page):
        AcronymToken(parent, acronym=info['subcommand'])
        return parent

class AcronymComponent(command.CommandComponent):
    COMMAND = 'ac'
    SUBCOMMAND = None

    def createToken(self, parent, info, page):
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
        settings['heading'] = (True, "Display the headings row of the acronym table.")
        settings['prefix'] = (u'Table', "Prefix to use when a caption and id are provided.")
        settings['caption'] = (None, "The caption to use for the acronym table.")
        return settings

    def createToken(self, parent, info, page):
        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  **self.attributes)
        acro = AcronymListToken(flt,
                                complete=self.settings['complete'],
                                heading=self.settings['heading'])
        if flt is parent:
            acro.attributes.update(**self.attributes)

        return parent

class RenderAcronymToken(components.RenderComponent):

    def _createSpan(self, parent, token, page):
        acro = self.extension.getAcronym(token['acronym'])
        content = str(acro.key) if acro.used else u'{} ({})'.format(acro.name, acro.key)
        return html.Tag(parent, 'span', string=content), acro

    def createHTML(self, parent, token, page):
        self._createSpan(parent, token, page)

    def createMaterialize(self, parent, token, page):
        span, acro = self._createSpan(parent, token, page)
        if acro.used:
            span.addClass('tooltipped')
            span['data-tooltip'] = acro.name
            span['data-position'] = 'top'
            span['data-delay'] = 50

    def createLatex(self, parent, token, page):
        acro = self.extension.getAcronym(token['acronym'])
        content = str(acro.key) if acro.used else u'{} ({})'.format(acro.name, acro.key)
        latex.String(parent, content=content)

class RenderAcronymListToken(components.RenderComponent):

    def createHTML(self, parent, token, page):
        rows = []
        for key, value in self.extension.getAcronyms(True).items():
            rows.append([key, value])

        heading = ['Acronym', 'Description'] if token['heading'] else None
        tbl = table.builder(rows, heading)
        self.renderer.render(parent, tbl, page)

    def createLatex(self, parent, token, page):

        env = latex.Environment(parent, 'tabulary',
                                args=[latex.Brace(None, string=u'\\linewidth', escape=False),
                                      latex.Brace(None, string=u'LL')])

        for key, value in self.extension.getAcronyms(True).items():
            latex.String(env, content=u'{}&{}\\\\'.format(key, value), escape=False)
