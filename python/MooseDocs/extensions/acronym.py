#pylint: disable=missing-docstring
import collections

from MooseDocs.base import components
from MooseDocs.common import exceptions
from MooseDocs.extensions import command, table, floats
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return AcronymExtension(**kwargs)

AcronymItem = collections.namedtuple('AcronymItem', 'key name used')

class AcronymToken(tokens.Token):
    """Token used for inline acronym use."""
    PROPERTIES = [tokens.Property('acronym', required=True, ptype=unicode)]

class AcronymListToken(tokens.Token):
    """Token for acronym lists."""
    PROPERTIES = [tokens.Property('complete', default=False, ptype=bool),
                  tokens.Property('heading', default=True, ptype=bool)]

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
        for key, value in self.get('acronyms').iteritems(): #pylint: disable=no-member
            if isinstance(value, dict):
                self.__acronyms.update(value)
            else:
                self.__acronyms[key] = value

    def reinit(self):
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
        self.addCommand(AcronymComponent())
        self.addCommand(AcronymListComponent())
        renderer.add(AcronymToken, RenderAcronymToken())
        renderer.add(AcronymListToken, RenderAcronymListToken())

class AcronymComponent(command.CommandComponent):
    COMMAND = 'acro'
    SUBCOMMAND = '*'

    def createToken(self, info, parent):
        AcronymToken(parent, acronym=info['subcommand'])
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

    def createToken(self, info, parent):
        flt = floats.create_float(parent, self.extension, self.settings, **self.attributes)
        AcronymListToken(flt,
                         complete=self.settings['complete'],
                         heading=self.settings['heading'])
        return parent

class RenderAcronymToken(components.RenderComponent):

    def _createSpan(self, token, parent):
        acro = self.extension.getAcronym(token.acronym)
        if acro is None:
            msg = "The acronym '{}' was not found."
            raise exceptions.RenderException(token.info, msg, token.acronym)

        content = unicode(acro.key) if acro.used else u'{} ({})'.format(acro.name, acro.key)
        return html.Tag(parent, 'span', string=content), acro

    def createHTML(self, token, parent):
        self._createSpan(token, parent)

    def createMaterialize(self, token, parent):
        span, acro = self._createSpan(token, parent)
        if acro.used:
            span.addClass('tooltipped')
            span['data-tooltip'] = acro.name
            span['data-position'] = 'top'
            span['data-delay'] = 50

    def createLatex(self, token, parent):
        pass

class RenderAcronymListToken(components.RenderComponent):

    def createHTML(self, token, parent):
        rows = []
        for key, value in self.extension.getAcronyms(token.complete).iteritems():
            rows.append([key, value])

        heading = ['Acronym', 'Description'] if token.heading else None
        tbl = table.builder(rows, heading)
        self.translator.renderer.process(parent, tbl)

    def createLatex(self, token, parent):
        pass
