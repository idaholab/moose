#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import uuid
import logging

import anytree

from pybtex.plugin import find_plugin, PluginNotFound
from pybtex.database import BibliographyData, parse_file
from pybtex.database.input.bibtex import UndefinedMacro, Person
from pylatexenc.latex2text import LatexNodes2Text

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.tree import tokens, html, latex
from MooseDocs.extensions import core, command

LOG = logging.getLogger('MooseDocs.extensions.bibtex')

def make_extension(**kwargs):
    return BibtexExtension(**kwargs)

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
        self.__citations = list()

    def initMetaData(self, page, meta):
        meta.initData('citations', list())

    def addCitations(self, *args):
        self.__citations.extend(args)

    def preExecute(self, content):

        duplicates = self.get('duplicates', list())
        self.__database = BibliographyData()

        bib_files = []
        for node in content:
            if node.source.endswith('.bib'):
                bib_files.append(node.source)

        for bfile in bib_files:
            try:
                db = parse_file(bfile)
            except UndefinedMacro as e:
                msg = "The BibTeX file %s has an undefined macro:\n%s"
                LOG.warning(msg, bfile, e)

            #TODO: https://bitbucket.org/pybtex-devs/pybtex/issues/93/
            #      databaseadd_entries-method-not-considering
            warn = self.get('duplicate_warning')
            for key in db.entries:
                duplicate_key = key in self.__database.entries
                duplicate_key_allowed = key in duplicates
                if duplicate_key and (not duplicate_key_allowed):
                    if warn:
                        msg = "The BibTeX entry '%s' defined in %s already exists."
                        LOG.warning(msg, key, bfile)
                elif not duplicate_key:
                    self.__database.add_entry(key, db.entries[key])

    def postTokenize(self, ast, page, meta, reader):
        if self.__citations:
            meta.getData('citations').extend(self.__citations)
            del self.__citations[:] # TODO: In python 3 this should be a clear()

            has_bib = False
            for node in anytree.PreOrderIter(ast):
                if node.name == 'BibtexBibliography':
                    has_bib = True
                    break

            if not has_bib:
                BibtexBibliography(ast)

    @property
    def database(self):
        return self.__database

    def extend(self, reader, renderer):
        self.requires(core, command)

        self.addCommand(reader, BibtexCommand())
        self.addCommand(reader, BibtexReferenceComponent())

        renderer.add('BibtexCite', RenderBibtexCite())
        renderer.add('BibtexBibliography', RenderBibtexBibliography())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('natbib', 'round')

BibtexCite = tokens.newToken('BibtexCite', keys=[])
BibtexBibliography = tokens.newToken('BibtexBibliography', bib_style=u'')
class BibtexReferenceComponent(command.CommandComponent):
    COMMAND = ('cite', 'citet', 'citep', 'nocite')
    SUBCOMMAND = None

    def createToken(self, parent, info, page):
        keys = [key.strip() for key in info['inline'].split(',')]
        BibtexCite(parent, keys=keys, cite=info['command'])
        self.extension.addCitations(*keys)
        return parent

class BibtexCommand(command.CommandComponent):
    COMMAND = 'bibtex'
    SUBCOMMAND = 'bibliography'

    @staticmethod
    def defaultSettings():
        config = command.CommandComponent.defaultSettings()
        config['style'] = (u'plain', "The BibTeX style (plain, unsrt, alpha, unsrtalpha).")
        config['title'] = (u'References', "The section title for the references.")
        config['title-level'] = (2, "The heading level for the section title for the references.")
        return config

    def createToken(self, parent, token, page): #pylint: disable=unused-argument
        if self.settings['title']:
            h = core.Heading(parent, level=int(self.settings['title-level']))
            self.reader.tokenize(h, self.settings['title'], page, MooseDocs.INLINE)
        BibtexBibliography(parent, bib_style=self.settings['style'])
        return parent

class RenderBibtexCite(components.RenderComponent):

    def createHTML(self, parent, token, page):

        cite = token['cite']
        if cite == 'nocite':
            return parent

        citep = cite == 'citep'
        if citep:
            html.String(parent, content=u'(')

        num_keys = len(token['keys'])
        for i, key in enumerate(token['keys']):

            if key not in self.extension.database.entries:
                LOG.error('Unknown BibTeX key: %s', key)
                html.Tag(parent, 'span', string=key, style='color:red;')
                continue


            entry = self.extension.database.entries[key]
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

            form = u'{}, {}' if citep else u'{} ({})'
            html.Tag(parent, 'a', href='#{}'.format(key),
                     string=form.format(author, entry.fields['year']))

            if citep:
                if num_keys > 1 and i != num_keys - 1:
                    html.String(parent, content=u'; ')
            else:
                if num_keys == 2 and i == 0:
                    html.String(parent, content=u' and ')
                elif num_keys > 2 and i == num_keys - 2:
                    html.String(parent, content=u', and ')
                elif num_keys > 2 and i != num_keys - 1:
                    html.String(parent, content=u', ')

        if citep:
            html.String(parent, content=u')')

        return parent

    def createMaterialize(self, parent, token, page):
        self.createHTML(parent, token, page)

    def createLatex(self, parent, token, page):
        latex.Command(parent, token['cite'], string=u','.join(token['keys']), escape=False)
        return parent

class RenderBibtexBibliography(components.RenderComponent):
    def createHTML(self, parent, token, page):

        try:
            style = find_plugin('pybtex.style.formatting', token['bib_style'])
        except PluginNotFound:
            msg = 'Unknown bibliography style "{}".'
            raise exceptions.MooseDocsException(msg, token['bib_style'])

        citations = list(self.translator.getMetaData(page, 'citations'))
        formatted_bibliography = style().format_bibliography(self.extension.database, citations)
        html_backend = find_plugin('pybtex.backends', 'html')

        div = html.Tag(parent, 'div', class_='moose-bibliography')
        ol = html.Tag(div, 'ol')

        backend = html_backend(encoding='utf-8')
        for entry in formatted_bibliography:
            text = entry.text.render(backend)
            html.Tag(ol, 'li', id_=entry.key, string=text)

        return ol

    def createMaterialize(self, parent, token, page):
        ol = self.createHTML(parent, token, page)
        for child in ol.children:
            key = child['id']
            db = BibliographyData()
            db.add_entry(key, self.extension.database.entries[key])
            btex = db.to_string("bibtex")

            m_id = uuid.uuid4()
            html.Tag(child, 'a',
                     style="padding-left:10px;",
                     class_='modal-trigger moose-bibtex-modal',
                     href="#{}".format(m_id),
                     string=u'[BibTeX]')

            modal = html.Tag(child, 'div', class_='modal', id_=m_id)
            content = html.Tag(modal, 'div', class_='modal-content')
            pre = html.Tag(content, 'pre', style="line-height:1.25;")
            html.Tag(pre, 'code', class_='language-latex', string=btex)

        return ol

    def createLatex(self, parent, token, page):
        pass
