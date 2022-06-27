#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import re
import uuid
import logging
import moosetree
from .. import common
from ..base import components, renderers
from ..tree import tokens, html, latex
from . import command, core, heading

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """Create an instance of the Extension object."""
    return KatexExtension(**kwargs)

Equation = tokens.newToken('Equation', tex=r'', inline=False, label=None, number=None, bookmark=None)
EquationReference = tokens.newToken('EquationReference', label=None, filename=None)

class KatexExtension(command.CommandExtension):
    """
    Extension object for parsing and rendering LaTeX equations with KaTeX.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Eq.', r"The prefix to used when referring to an equation by " \
                                   r"the \\label content.")
        config['macros'] = (None, "Macro definitions to apply to equations.")
        return config

    def extend(self, reader, renderer):
        """
        Add the necessary components for reading and rendering LaTeX.
        """
        self.requires(core, heading)
        self.addCommand(reader, EquationCommand())
        self.addCommand(reader, EquationReferenceCommand())

        renderer.add('Equation', RenderEquation())
        renderer.add('EquationReference', RenderEquationReference())

        # Deprecated
        reader.addInline(KatexBlockEquationComponent(), location='_begin')
        reader.addInline(KatexInlineEquationComponent(), location='_begin')
        renderer.add('ShortcutLink', RenderEquationLink())

        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage('amsfonts')
            if self.get('macros', None):
                for k, v in self.get('macros').items():
                    renderer.addNewCommand(k, v)

    def initPage(self, page):

        # Contains a mapping of the equation label ("id=") to a tuple that contains
        # the equation number and unique bookmark. This used for equation referencing.
        page['labels'] = dict()

    def postTokenize(self, page, ast):
        labels = dict()
        count = 0
        func = lambda n: (n.name == 'Equation') and (n['label'] is not None)
        for node in moosetree.iterate(ast, func):
            count += 1
            node['number'] = count
            labels[node['label']] = (count, node['bookmark'])

            # TODO: When !eqref is used for references, this should be removed
            core.Shortcut(ast,
                          key=node['label'],
                          string='{} ({})'.format(self.get('prefix'), count),
                          link='#{}'.format(node['bookmark']))

        page['labels'] = labels

        renderer = self.translator.renderer
        if common.has_tokens(ast, 'Equation') and isinstance(renderer, renderers.HTMLRenderer):
            renderer.addCSS('katex', "contrib/katex/katex.min.css", page)
            renderer.addCSS('katex_moose', "css/katex_moose.css", page)
            renderer.addJavaScript('katex', "contrib/katex/katex.min.js", page, head=True)

            if self.get('macros', None):
                mc = ','.join('"{}":"{}"'.format(k, v) for k, v in self.get('macros').items())
                self.setAttribute('macros', '{' + mc + '}')

class EquationCommand(command.CommandComponent):
    COMMAND = ('equation', 'eq')
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['id'] = (None, "The equation label for referencing within text, if provided " \
                                 "the equation is numbered.")
        return settings

    def createToken(self, parent, info, page, settings):
        inline = 'inline' in info
        if inline and info['command'] == 'equation':
            raise common.exceptions.MooseDocsException("The '!equation' command is a block level command, use '!eq' instead.")
        if not inline and info['command'] == 'eq':
            raise common.exceptions.MooseDocsException("The '!eq' command is an inline level command, use '!equation' instead.")
        if inline and (settings.get('id', None) is not None):
            raise common.exceptions.MooseDocsException("The 'id' setting is not allowed within in the inline equation command.")

        # Extract the TeX
        tex = info['inline'] if inline else info['block']
        tex = r'{}'.format(tex.strip('\n').replace('\n', ' '))

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Build the token
        Equation(parent, tex=tex, bookmark=eq_id, label=settings['id'], inline=inline)
        return parent

class KatexBlockEquationComponent(components.ReaderComponent):
    """
    Component for reading LaTeX block equations.
    """
    RE = re.compile(r'^\\begin{(?P<cmd>equation\*{0,1})}' # start equation block
                    r'(?P<equation>.*?)'                  # tex equation
                    r'^\\end{(?P=cmd)}',                  # end equation block
                    flags=re.DOTALL|re.MULTILINE|re.UNICODE)
    LABEL_RE = re.compile(r'\\label{(?P<id>.*?)}', flags=re.UNICODE)

    def createToken(self, parent, info, page, settings):
        """Create a LatexBlockEquation token."""

        # TODO: Change to new syntax
        #msg = '{}:{}\n'.format(page.source, info.line)
        #msg += "The LaTeX style commands for defining equations is deprecated, please update " \
        #       "your markdown to use the !equation command."
        #LOG.warning(msg)

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ')

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Build the token
        token = Equation(parent, tex=tex, bookmark=eq_id)

        # Add a label
        label = self.LABEL_RE.search(info['equation'])
        if label:
            token['label'] = label.group('id')
            token['tex'] = token['tex'].replace(label.group(), '')
        return parent

class KatexInlineEquationComponent(components.ReaderComponent):
    RE = re.compile(r'(?P<token>\$)(?=\S)(?P<equation>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, parent, info, page, settings):
        """Create LatexInlineEquation"""

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ')

        # TODO: Change to new syntax
        # msg = '{}:{}\n'.format(page.source, info.line)
        # msg += "The LaTeX style commands for defining equations is deprecated, please update " \
        #        "your markdown to use the [!eq]({}) command.".format(tex)
        # LOG.warning(msg)

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Create token
        Equation(parent, tex=tex, bookmark=eq_id, inline=True)
        return parent

class EquationReferenceCommand(command.CommandComponent):
    COMMAND = 'eqref'
    SUBCOMMAND = None
    LABEL_RE = re.compile(r'((?P<filename>.*?\.md)#)?(?P<label>.+)', flags=re.UNICODE)

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        inline = 'inline' in info
        if not inline:
            raise common.exceptions.MooseDocsException("The '!eqref' command is an inline level command.")

        content = info['inline']
        match = self.LABEL_RE.search(content)
        if match is None:
            raise common.exceptions.MooseDocsException("Invalid equation label format.")

        EquationReference(parent, label=match.group('label'), filename=match.group('filename'))
        return parent

class RenderEquation(components.RenderComponent):
    def createHTML(self, parent, token, page):

        if token['inline']:
            div = html.Tag(parent, 'span', token, class_='moose-katex-inline-equation',
                           id_=token['bookmark'])
            display = 'false'

        else:
            # Wrap all equation related items in an outer div
            div = html.Tag(parent, 'span', class_='moose-katex-block-equation')
            display = 'true'

            # Create equation content and number (if it is valid)
            html.Tag(div, 'span', token, class_='moose-katex-equation table-cell',
                     id_=token['bookmark'])
            if token['label'] is not None:
                num = html.Tag(div, 'span', class_='moose-katex-equation-number')
                html.String(num, content='({})'.format(token['number']))

        # Build the KaTeX script
        script = html.Tag(div, 'script')
        config = dict()
        config['displayMode'] = display
        config['throwOnError'] = 'false'

        macros = self.extension.getAttribute('macros', None)
        if macros:
            config['macros'] = macros

        config_str = ','.join('{}:{}'.format(key, value) for key, value in config.items())

        config_str = config_str.encode('unicode_escape').decode('utf-8')
        tex = token['tex'].encode('unicode_escape').decode('utf-8')

        content = 'var element = document.getElementById("%s");' % token['bookmark']
        content += 'katex.render("%s", element, {%s});' % \
                   (tex, config_str)
        html.String(script, content=content)

        return parent

    def createLatex(self, parent, token, page):
        if token['inline']:
            latex.String(parent, content='${}$'.format(token['tex']), escape=False)
        else:
            cmd = 'equation' if token['number'] else 'equation*'
            env = latex.Environment(parent, cmd)
            if token['label']:
                latex.Command(env, 'label', string=token['label'], end='\n', escape=False)
            latex.String(env, content=token['tex'], escape=False)

        return parent

class RenderEquationLink(core.RenderShortcutLink):

    def createLatex(self, parent, token, page):
        key = token['key']
        if key in page.get('labels'):
            latex.String(parent, content=self.extension['prefix'] + '~', escape=False)
            latex.Command(parent, 'eqref', string=key, escape=False)
            return parent
        return core.RenderShortcutLink.createLatex(self, parent, token, page)

class RenderEquationReference(core.RenderShortcutLink):

    def createHTML(self, parent, token, page):
        a = html.Tag(parent, 'a', class_='moose-equation-reference')
        eq_page = page

        if token['filename']:
            eq_page = self.translator.findPage(token['filename'])
            head = heading.find_heading(eq_page)
            if head is not None:
                tok = tokens.Token(None)
                head.copyToToken(tok)
                self.renderer.render(a, tok, page)
                html.String(a, content=', ')
            else:
                html.String(a, content=token['filename'] + ', ')

        num, id_ = eq_page['labels'].get(token['label'], (None, None))
        # TODO: Error if label not found
        if eq_page is not page:
            url = eq_page.relativeDestination(page)
            a['href']='{}#{}'.format(url, id_)
        else:
            a['href']='#{}'.format(id_)

        if num is None:
            a['class'] = 'moose-error'
            html.String(a, content='{}#{}'.format(eq_page.local, token['label']))
            msg = "Could not find equation with key {} on page {}".format(token['label'], eq_page.local)
            raise common.exceptions.MooseDocsException(msg)
        else:
            html.String(a, content='{} ({})'.format(self.extension['prefix'], num))

    def createLatex(self, parent, token, page):
        key = token['label']
        latex.String(parent, content=self.extension['prefix'] + '~', escape=False)
        latex.Command(parent, 'eqref', string=key, escape=False)
        return parent
