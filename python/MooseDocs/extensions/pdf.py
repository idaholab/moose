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
import subprocess
import logging
import collections
import moosetree
import mooseutils
from ..base import renderers
from ..common import exceptions, box
from ..tree import base, latex, pages
from . import core, command, bibtex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """Create an instance of the Extension object."""
    return PDFExtension(**kwargs)

class PDFExtension(command.CommandExtension):
    """
    Extension to build a single PDF using latex.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        """
        Add the necessary components for reading and rendering LaTeX.
        """
        self.requires(core, command, bibtex)

        if not isinstance(renderer, renderers.LatexRenderer):
            self.setActive(False)
        else:
            renderer.addPackage('geometry', margin='1in')
            renderer.addPackage('parskip')

    def postTokenize(self, page, ast):
        """
        Performs modification of heading level base on the local folder depth.
        """

        depth = page.depth if page.depth < 2 else 2
        for node in moosetree.iterate(ast):

            if node.name == 'Heading':
                lvl = node['level'] + depth
                node['level'] = lvl

                if lvl > 6:
                    msg = '{}:{}\n'.format(page.source, node.info.line)
                    msg += "The heading with the following text is greater than level 4, which " \
                           "should be avoided if you plan on using the pdf extension with the " \
                           "LatexRenderer. The level is being set to 4. Please see " \
                           "www.mooseframework.org for details."
                    msg += '\n  %s'
                    LOG.warning(msg, node.text())
                    node['level'] = 4

    def postExecute(self):
        """
        Combines all the LaTeX files into a single file.
        """
        return

        files = []
        for page in self.translator.getPages():
            if isinstance(page, pages.Source):
                files.append(page)
        root = self.buildTreeFromPageList(files)

        main = self._processPages(root)

        loc = self.translator['destination']
        with open(os.path.join(loc, 'main.tex'), 'w+') as fid:
            fid.write(main.write())

        main_tex = os.path.join(loc, 'main.tex')
        LOG.info("Building complete LaTeX document: %s", main_tex)
        commands = [['pdflatex', '-halt-on-error', 'main'],
                    ['bibtex', 'main'],
                    ['pdflatex', '-halt-on-error', 'main'],
                    ['pdflatex', '-halt-on-error', 'main']]
        for cmd in commands:
            try:
                output = subprocess.check_output(cmd, cwd=loc, stderr=subprocess.STDOUT, encoding='utf8')
            except subprocess.CalledProcessError as e:
                msg = 'Failed to run pdflatex command: {}\n\n{}'
                raise exceptions.MooseDocsException(msg, ' '.join(cmd), e.output)

        # Process output
        root = self.processLatexOutput(output, content)
        for node in moosetree.iterate(root):
            if node['warnings']:
                self._reportLatexWarnings(node, content)

    def _processPages(self, root):
        """
        Build a main latex file that includes the others.
        """

        main = base.NodeBase(None, None)
        latex.Command(main, 'documentclass', string='report', end='')
        for package, options in self.translator.renderer.getPackages().items():
            args = []
            if options[0] or options[1]:
                args = [self._getOptions(*options)]

            latex.Command(main, 'usepackage', args=args, string=package, start='\n', end='')

        latex.String(main, content='\\setlength{\\parindent}{0pt}', start='\n', escape=False)

        for preamble in self.translator.renderer.getPreamble():
            latex.String(main, content='\n' + preamble, escape=False)

        # New Commands
        for cmd in self.translator.renderer.getNewCommands().values():
            cmd.parent = main

        doc = latex.Environment(main, 'document', end='\n')
        for node in moosetree.iterate(root, lambda n: 'page' in n):
            page = node['page']
            if page.get('active', True):
                cmd = latex.Command(doc, 'input', start='\n')
                latex.String(cmd, content=str(page.destination), escape=False)

        # BibTeX
        bib_files = [n.source for n in self.translator.getPages() if n.source.endswith('.bib')]
        if bib_files:
            latex.Command(doc, 'bibliographystyle', start='\n', string='unsrtnat')
            latex.Command(doc, 'bibliography', string=','.join(bib_files), start='\n',
                          escape=False)

        return main

    def _reportLatexWarnings(self, lnode, content):
        """Helper to display latex warnings."""

        # Locate the Page object where the error was producec.
        pnode = None
        for page in content:
            if lnode['filename'] in page.destination:
                pnode = page
                break

        # Get the rendered result tree and locate the start/end lines for each node
        result = None
        if pnode is not None:
            pass
            # TODO: The ability to get the ResultTree has been removed, this was the only function
            #       calling it and the potential slow downs and abuse out pace the use case here
            #result = self.translator.getResultTree(pnode)
            #result['_start_line'] = 1
            #self._lineCounter(result)

        # Report warning(s)
        for w in lnode['warnings']:

            # Locate the rendered node that that caused the error
            r_node = None
            if result:
                for r in moosetree.iterate(result):
                    if w.line >= r.get('_start_line', float('Inf')):
                        r_node = r

            # Build message
            msg = '\n'
            msg += mooseutils.colorText('pdfLaTeX Warning: {}\n'.format(w.content),
                                        'LIGHT_YELLOW')

            if r_node:
                msg += box(r_node.write(),
                           title='IN: {}:{}'.format(pnode.destination, w.line),
                           width=100,
                           color='GREY')
                msg += '\n'

                info = r_node.get('info', None)
                if info is not None:
                    msg += box(info[0],
                               title='FROM: {}:{}'.format(pnode.source, info.line),
                               color='GREY')

            LOG.warning(msg)

    @staticmethod
    def _lineCounter(node):
        """Helper to locate the start/end lines of the write() content."""
        cnt = node['_start_line']
        for child in node:
            if isinstance(child, latex.String):
                continue
            out = child.write()
            cnt += out.count('\n')
            child['_start_line'] = cnt - out.strip('\n').count('\n')
            child['_end_line'] = cnt
            PDFExtension._lineCounter(child)

    @staticmethod
    def _getOptions(args, kwargs):
        txt = []
        for key in args:
            txt.append(key)
        for key, value in kwargs.items():
            txt.append('{}={}'.format(key, str(value)))
        return latex.Bracket(string=','.join(txt), escape=False)

    @staticmethod
    def processLatexOutput(output, nodes):
        """Convert the pdfLatex log into a tree structure to capture the warnings.

        Inputs:
            output[str]: The console output or the pdflatex log.
            content[list]: The list of page objects from the translator.
        """

        # Items for capturing errors
        warn = collections.namedtuple('LatexWarning', 'content line')
        line_re = re.compile(r'line\s(?P<line>[0-9]+)')
        regex = re.compile(r'^(?P<filename>\.*[^\.]+\.\w+)(?P<content>.*)',
                           flags=re.MULTILINE|re.DOTALL)

        # Convert output to a tree structure
        root = PDFExtension.parseOutput(output)

        # Loop through the result and capture filenames and warnings
        for n in moosetree.iterate(root):
            match = regex.search(n['content'])
            if match:
                n['content'] = match.group('content')
                n['filename'] = match.group('filename').replace('\n', '')
            n['content'] = [c.strip().replace('\n', '') for c in re.split(r'\n{2,}', n['content']) if c]

            for c in n['content']:
                if 'LaTeX Warning' in c:
                    c = c.replace('LaTeX Warning:', '').strip()
                    match = line_re.search(c)
                    line = int(match.group('line')) if match else None
                    n['warnings'].append(warn(content=c, line=line))

        return root

    @staticmethod
    def parseOutput(content):
        """Convert the nested parenthesis output from pdflatex to a tree structure."""
        root = moosetree.Node(None, 'root', content='', filename='', warnings=[])
        node = root
        for char in content:
            if char == '(':
                node = moosetree.Node(node, 'paren', content='', filename='', warnings=[])
            elif char == ')':
                node = node.parent
            else:
                node['content'] += str(char)
        return root

    @staticmethod
    def buildTreeFromPageList(files):
        """Create a tree structure from the supplied page objects."""

        tree = dict()
        root = base.NodeBase('', None)
        tree[('',)] = root

        # Sort
        nodes = sorted(files, key=lambda f: f.local)

        # Build directories
        for node in nodes:
            key = tuple([''] + node.local.split(os.sep))[:-1]
            for i in range(1, len(key) + 1):
                k = key[:i]
                if k not in tree:
                    tree[k] = base.NodeBase(k[-1], tree[k[:-1]])

        # Insert pages
        for node in nodes:
            key = tuple([''] + node.local.split(os.sep))
            if key[-1] == 'index.md':
                tree[key[:-1]]['page'] = node
            else:
                base.NodeBase(key[-1], tree[key[:-1]], page=node)

        def sort_files(node):
            """Helper for sorting files before folders."""
            for child in reversed(node.children):
                if not child.children:
                    child.parent = None
                    node.insert(0, child)
                else:
                    sort_files(child)

        sort_files(root)
        return root
