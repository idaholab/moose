#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import uuid
import logging

from pybtex.plugin import find_plugin, PluginNotFound
from pybtex.database import BibliographyData, parse_file
from pybtex.database.input.bibtex import UndefinedMacro, Person
from pybtex.errors import set_strict_mode
from pylatexenc.latex2text import LatexNodes2Text

import moosetree

from ..common import exceptions
from ..base import components, LatexRenderer, MarkdownReader
from ..tree import tokens, html, latex
from . import core, command

LOG = logging.getLogger('MooseDocs.extensions.bibtex')

def make_extension(**kwargs):
    return BibtexExtension(**kwargs)

BibtexCite = tokens.newToken('BibtexCite', keys=[])
BibtexBibliography = tokens.newToken('BibtexBibliography', bib_style='')
BibtexList = tokens.newToken('BibtexList', BibtexBibliography, bib_files=None)

class BibtexExtension(command.CommandExtension):
    """
    Extension for BibTeX citations and bibliography.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['duplicate_warning'] = (True, "Show a warning when duplicate entries detected.")
        config['duplicates'] = (list(), "A list of duplicates that are allowed.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        self.__database = None
        self.__bib_files = list()
        self.__bib_file_database = dict()

    def preExecute(self):
        set_strict_mode(False) # allow incorrectly formatted author/editor names

        # If this is invoked during a live serve, we need to recompile the list of '.bib' files and
        # read them again, otherwise there's no way to distinguish existing entries from duplicates
        self.__bib_files = []
        for node in self.translator.findPages(lambda p: p.source.endswith('.bib')):
            self.__bib_files.append(node.source)

        self.__database = BibliographyData()
        for bfile in self.__bib_files:
            try:
                db = parse_file(bfile)
                self.__bib_file_database[bfile] = db
            except UndefinedMacro as e:
                msg = "The BibTeX file %s has an undefined macro:\n%s"
                LOG.warning(msg, bfile, e)

            #TODO: https://bitbucket.org/pybtex-devs/pybtex/issues/93/
            #      databaseadd_entries-method-not-considering
            for key in db.entries:
                if key in self.__database.entries:
                    if self.get('duplicate_warning') and (key not in self.get('duplicates')):
                        msg = "The BibTeX entry '%s' defined in %s already exists."
                        LOG.warning(msg, key, bfile)
                else:
                    self.__database.add_entry(key, db.entries[key])

    def preRead(self, page):
        """Initialize the page citations list."""
        page['citations'] = list()

    def postTokenize(self, page, ast):
        if page['citations']:
            has_bib = False
            for node in moosetree.iterate(ast):
                if node.name == 'BibtexBibliography':
                    has_bib = True
                    break

            if not has_bib:
                core.Heading(ast, level=2, string='References')
                BibtexBibliography(ast, bib_style='plain')

    def database(self, bibfile=None):
        if bibfile is None:
            return self.__database
        else:
            return self.__bib_file_database[bibfile]

    def bibfiles(self):
        return self.__bib_files

    def extend(self, reader, renderer):
        self.requires(core, command)

        self.addCommand(reader, BibtexCommand())
        self.addCommand(reader, BibtexListCommand())
        self.addCommand(reader, BibtexReferenceComponent())

        renderer.add('BibtexCite', RenderBibtexCite())
        renderer.add('BibtexList', RenderBibtexList())
        renderer.add('BibtexBibliography', RenderBibtexBibliography())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('natbib', 'round')

class BibtexReferenceComponent(command.CommandComponent):
    COMMAND = ('cite', 'citet', 'citep', 'nocite')
    SUBCOMMAND = None

    def createToken(self, parent, info, page, settings):
        keys = [key.strip() for key in info['inline'].split(',')]
        BibtexCite(parent, keys=keys, cite=info['command'])
        page['citations'].extend(keys)
        return parent

class BibtexCommand(command.CommandComponent):
    COMMAND = 'bibtex'
    SUBCOMMAND = 'bibliography'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['style'] = ('plain', "The BibTeX style (plain, unsrt, alpha, unsrtalpha).")
        config['title'] = ('References', "The section title for the references.")
        config['title-level'] = (2, "The heading level for the section title for the references.")
        return config

    def createToken(self, parent, token, page, settings):
        if settings['title']:
            h = core.Heading(parent, level=int(settings['title-level']))
            self.reader.tokenize(h, settings['title'], page, MarkdownReader.INLINE)
        BibtexBibliography(parent, bib_style=settings['style'])
        return parent

class BibtexListCommand(command.CommandComponent):
    COMMAND = 'bibtex'
    SUBCOMMAND = 'list'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['bib_files'] = (None, "The list of *.bib files to use for a complete citation list.")
        return config

    def createToken(self, parent, token, page, settings):
        bfiles = settings['bib_files']
        bib_files = list()
        if bfiles is None:
            bib_files = self.extension.bibfiles()
        else:
            for bfile in bfiles.split():
                for key in self.extension.bibfiles():
                    if key.endswith(bfile):
                        bib_files.append(key)

        BibtexList(parent, bib_files=bib_files)
        return parent

class RenderBibtexCite(components.RenderComponent):

    def createHTML(self, parent, token, page):

        cite = token['cite']
        if cite == 'nocite':
            return parent

        citep = cite == 'citep'
        if citep:
            html.String(parent, content='(')

        num_keys = len(token['keys'])
        for i, key in enumerate(token['keys']):

            if key not in self.extension.database().entries:
                LOG.error('Unknown BibTeX key: %s', key)
                html.Tag(parent, 'span', string=key, style='color:red;')
                continue


            entry = self.extension.database().entries[key]
            author_found = True
            if not 'author' in entry.persons.keys() and not 'Author' in entry.persons.keys():
                author_found = False
                entities = ['institution', 'organization']
                for entity in entities:
                    if entity in entry.fields.keys():
                        author_found = True
                        name = ''
                        for word in entry.fields[entity]:
                            if word[0].isupper():
                                name += word[0]
                        entry.persons['author'] = [Person(name)]

            if not author_found:
                msg = 'No author, institution, or organization for {}'
                raise exceptions.MooseDocsException(msg, key)

            a = entry.persons['author']
            n = len(a)
            if n > 2:
                author = '{} et al.'.format(' '.join(a[0].last_names))
            elif n == 2:
                a0 = ' '.join(a[0].last_names)
                a1 = ' '.join(a[1].last_names)
                author = '{} and {}'.format(a0, a1)
            else:
                author = ' '.join(a[0].last_names)

            author = LatexNodes2Text().latex_to_text(author)

            form = '{}, {}' if citep else '{} ({})'
            year = entry.fields.get('year', None)
            if year is None:
                raise exceptions.MooseDocsException("Unable to locate year for bibtex entry '{}'", entry.key)
            html.Tag(parent, 'a', href='#{}'.format(key), string=form.format(author, year))

            if citep:
                if num_keys > 1 and i != num_keys - 1:
                    html.String(parent, content='; ')
            else:
                if num_keys == 2 and i == 0:
                    html.String(parent, content=' and ')
                elif num_keys > 2 and i == num_keys - 2:
                    html.String(parent, content=', and ')
                elif num_keys > 2 and i != num_keys - 1:
                    html.String(parent, content=', ')

        if citep:
            html.String(parent, content=')')

        return parent

    def createMaterialize(self, parent, token, page):
        self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):
        latex.Command(parent, token['cite'], string=','.join(token['keys']), escape=False)
        return parent

class RenderBibtexBibliography(components.RenderComponent):

    def getCitations(self, parent, token, page):
        return page.get('citations', list())

    def createHTML(self, parent, token, page):

        try:
            style = find_plugin('pybtex.style.formatting', token['bib_style'])
        except PluginNotFound:
            msg = 'Unknown bibliography style "{}".'
            raise exceptions.MooseDocsException(msg, token['bib_style'])

        citations = self.getCitations(parent, token, page)
        formatted_bibliography = style().format_bibliography(self.extension.database(), citations)

        if formatted_bibliography.entries:
            html_backend = find_plugin('pybtex.backends', 'html')
            div = html.Tag(parent, 'div', class_='moose-bibliography')
            ol = html.Tag(div, 'ol')

            backend = html_backend(encoding='utf-8')
            for entry in formatted_bibliography:
                text = entry.text.render(backend)
                html.Tag(ol, 'li', id_=entry.key, string=text)

            return ol

        else:
            html.String(parent, content="No citations exist within this document.")

    def createMaterialize(self, parent, token, page):
        ol = self.createHTML(parent, token, page)
        if ol is None:
            return

        for child in ol.children:
            key = child['id']
            db = BibliographyData()
            db.add_entry(key, self.extension.database().entries[key])
            btex = db.to_string("bibtex")

            m_id = uuid.uuid4()
            html.Tag(child, 'a',
                     style="padding-left:10px;",
                     class_='modal-trigger moose-bibtex-modal',
                     href="#{}".format(m_id),
                     string='[BibTeX]')

            modal = html.Tag(child, 'div', class_='modal', id_=m_id)
            content = html.Tag(modal, 'div', class_='modal-content')
            pre = html.Tag(content, 'pre', style="line-height:1.25;")
            html.Tag(pre, 'code', class_='language-latex', string=btex)

        return ol

    def createLatex(self, parent, token, page):
        pass

class RenderBibtexList(RenderBibtexBibliography):
    def getCitations(self, parent, token, page):
        citations = list()
        for bfile in token['bib_files']:
            citations += self.extension.database(bfile).entries.keys()
        return citations
