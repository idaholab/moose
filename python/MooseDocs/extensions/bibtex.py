#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import shutil
import os
import re
import io
import logging

from pybtex.plugin import find_plugin, PluginNotFound
from pybtex.database import BibliographyData, parse_file
from pybtex.database.input.bibtex import UndefinedMacro

import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon
from markdown.preprocessors import Preprocessor

LOG = logging.getLogger(__name__)

class BibtexExtension(MooseMarkdownExtension):
    """
    Extension for adding bibtex style references and bibliographies to MOOSE flavored markdown.
    """

    @staticmethod
    def defaultConfig():
        """BibtexExtension default configure options."""
        config = MooseMarkdownExtension.defaultConfig()
        config['macro_files'] = ['', "List of paths to files that contain macros to be used in " \
                                     " bibtex parsing."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds Bibtex support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose_bibtex',
                             BibtexPreprocessor(markdown_instance=md, **config), '_end')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create BibtexExtension"""
    return BibtexExtension(*args, **kwargs)

class BibtexPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    Creates per-page bibliographies using latex syntax.
    """

    RE_BIBLIOGRAPHY = r'(?<!`)\\bibliography\{(.*?)\}'
    RE_STYLE = r'(?<!`)\\bibliographystyle\{(.*?)\}'
    RE_CITE = r'(?<!`)\\(?P<cmd>cite|citet|citep)\{(?P<keys>.*?)\}'

    @staticmethod
    def defaultSettings():
        """BibtexPreprocessor configure options."""
        return dict() # this extension doesn't have settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Preprocessor.__init__(self, markdown_instance)
        self._macro_files = kwargs.pop('macro_files', None)
        self._bibtex = None
        self._citations = []

    def parseBibtexFile(self, bibfile):
        """
        Returns parsed bibtex file.  If "macro_files" are supplied in the configuration
        file, then a temporary file will be made that contains the supplied macros
        above the original bib file.  This temporary combined file can then be
        parsed by pybtex.
        """

        if self._macro_files:
            t_bib_path = MooseDocs.abspath("tBib.bib")
            with open(t_bib_path, "wb") as t_bib:
                for t_file in self._macro_files:
                    with open(MooseDocs.abspath(t_file.strip()), "rb") as in_file:
                        shutil.copyfileobj(in_file, t_bib)
                with open(bibfile, "rb") as in_file:
                    shutil.copyfileobj(in_file, t_bib)
            data = parse_file(t_bib_path)
            if os.path.isfile(t_bib_path):
                os.remove(t_bib_path)
        else:
            data = parse_file(bibfile)

        return data

    def run(self, lines):
        """
        Create a bibliography from cite commands.
        """

        # Join the content to enable regex searches throughout entire text
        content = '\n'.join(lines)

        # Build the database of bibtex data
        self._citations = []              # member b/c it is used in substitution function
        self._bibtex = BibliographyData() # ""
        bibfiles = []
        match = re.search(self.RE_BIBLIOGRAPHY, content)
        if match:
            for bfile in match.group(1).split(','):
                try:
                    bibfiles.append(MooseDocs.abspath(bfile.strip()))
                    data = self.parseBibtexFile(bibfiles[-1])
                except UndefinedMacro:
                    LOG.error('Undefined macro in bibtex file: %s, specify macro_files arguments ' \
                              'in configuration file (e.g. website.yml)', bfile.strip())
                self._bibtex.add_entries(data.entries.iteritems())
        else:
            return lines

        # Determine the style
        match = re.search(self.RE_STYLE, content)
        if match:
            content = content.replace(match.group(0), '')
            try:
                style = find_plugin('pybtex.style.formatting', match.group(1))
            except PluginNotFound:
                LOG.error('Unknown bibliography style "%s"', match.group(1))
                return lines

        else:
            style = find_plugin('pybtex.style.formatting', 'plain')

        # Replace citations with author date, as an anchor
        content = re.sub(self.RE_CITE, self.authors, content)

        # Create html bibliography
        if self._citations:

            # Generate formatted html using pybtex
            formatted_bibliography = style().format_bibliography(self._bibtex, self._citations)
            backend = find_plugin('pybtex.backends', 'html')
            stream = io.StringIO()
            backend().write_to_stream(formatted_bibliography, stream)

            # Strip the bib items from the formatted html
            html = re.findall(r'\<dd\>(.*?)\</dd\>', stream.getvalue(),
                              flags=re.MULTILINE|re.DOTALL)

            # Produces an ordered list with anchors to the citations
            output = u'<ol class="moose-bibliography" data-moose-bibfiles="{}">\n'
            output = output.format(str(bibfiles))
            for i, item in enumerate(html):
                output += u'<li name="{}">{}</li>\n'.format(self._citations[i], item)
            output += u'</ol>\n'
            content = re.sub(self.RE_BIBLIOGRAPHY,
                             self.markdown.htmlStash.store(output, safe=True),
                             content)

        return content.split('\n')

    def authors(self, match):
        """
        Return the author(s) citation for text, linked to bibliography.
        """
        cmd = match.group('cmd')
        keys = match.group('keys')
        tex = '\\%s{%s}' % (cmd, keys)

        cite_list = []

        # Loop over all keys in the cite command
        for key in [k.strip() for k in keys.split(',')]:

            # Error if the key is not found and move on
            if key not in self._bibtex.entries:
                LOG.error('Unknown bibtext key: %s', key)
                continue

            # Build the author list
            self._citations.append(key)
            entry = self._bibtex.entries[key]
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

            if cmd == 'citep':
                a = '<a href="#{}">{}, {}</a>'.format(key, author, entry.fields['year'])
            else:
                a = '<a href="#{}">{} ({})</a>'.format(key, author, entry.fields['year'])

            cite_list.append(a)

        # Create the correct text for list of keys in the cite command
        if len(cite_list) == 2:
            cite_list = [' and '.join(cite_list)]
        elif len(cite_list) > 2:
            cite_list[-1] = 'and ' + cite_list[-1]

        # Write the html
        if cmd == 'citep':
            html = '(<span data-moose-cite="{}">{}</span>)'.format(tex, '; '.join(cite_list))
        else:
            html = '<span data-moose-cite="{}">{}</span>'.format(tex, ', '.join(cite_list))

        # substitute Umlauts
        umlaut_re = re.compile(r"\{\\\"([aouAOU])\}")
        html = umlaut_re.sub('&\\1uml;', html)

        return self.markdown.htmlStash.store(html, safe=True)
