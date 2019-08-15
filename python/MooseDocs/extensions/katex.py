#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import uuid
import anytree
from MooseDocs.base import components, renderers
from MooseDocs.tree import tokens, html, latex
from MooseDocs.extensions import command, core, floats

def make_extension(**kwargs):
    """Create an instance of the Extension object."""
    return KatexExtension(**kwargs)

LatexBlockEquation = tokens.newToken('LatexBlockEquation',
                                     tex=r'',
                                     label=None,
                                     number=None,
                                     bookmark=None)
LatexInlineEquation = tokens.newToken('LatexInlineEquation', tex=r'', bookmark=None)

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

    def __init__(self, *args, **kwargs):
        super(KatexExtension, self).__init__(*args, **kwargs)
        self.macros = None

    def initMetaData(self, page, meta):
        meta.initData('labels', set())

    def extend(self, reader, renderer):
        """
        Add the necessary components for reading and rendering LaTeX.
        """
        self.requires(core, floats)
        self.addCommand(reader, KatexBlockEquationCommand())
        reader.addInline(KatexBlockEquationComponent(), location='_begin')
        reader.addInline(KatexInlineEquationComponent(), location='_begin')
        renderer.add('LatexBlockEquation', RenderLatexEquation())
        renderer.add('LatexInlineEquation', RenderLatexEquation())
        renderer.add('ShortcutLink', RenderEquationLink())

        if isinstance(renderer, renderers.HTMLRenderer):
            renderer.addCSS('katex', "contrib/katex/katex.min.css")
            renderer.addCSS('katex_moose', "css/katex_moose.css")
            renderer.addJavaScript('katex', "contrib/katex/katex.min.js", head=True)

            if self.get('macros', None):
                mc = ','.join('"{}":"{}"'.format(k, v) for k, v in self.get('macros').items()) #pylint: disable=no-member
                self.macros = '{' + mc + '}'

        elif isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage('amsfonts')
            if self.get('macros', None):
                for k, v in self.get('macros').items(): #pylint: disable=no-member
                    renderer.addNewCommand(k, v)

    def postTokenize(self, ast, page, meta, reader):
        labels = set()
        count = 0
        func = lambda n: (n.name == 'LatexBlockEquation') and (n['label'] is not None)
        for node in anytree.PreOrderIter(ast, filter_=func):
            count += 1
            node.set('number', count)
            if node['label']:
                if isinstance(self.translator.renderer, renderers.LatexRenderer):
                    labels.add(node['label'])
                else:
                    core.Shortcut(ast,
                                  key=node['label'],
                                  string='{} ({})'.format(self.get('prefix'), count),
                                  link=u'#{}'.format(node['bookmark']))

        meta.setData('labels', labels)

class KatexBlockEquationCommand(command.CommandComponent):
    COMMAND = 'equation'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['id'] = (None, "The equation label for referencing within text, if provided " \
                                 "the equation is numbered.")
        return settings

    def createToken(self, parent, info, page):

        # Extract the TeX
        tex = info['inline'] if 'inline' in info else info['block']
        tex = r'{}'.format(tex.strip('\n').replace('\n', ' '))

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Build the token
        LatexBlockEquation(parent, tex=tex, bookmark=eq_id, label=self.settings['id'])
        return parent

class KatexBlockEquationComponent(components.TokenComponent):
    """
    Component for reading LaTeX block equations.
    """
    RE = re.compile(r'^\\begin{(?P<cmd>equation\*{0,1})}' # start equation block
                    r'(?P<equation>.*?)'                  # tex equation
                    r'^\\end{(?P=cmd)}',                  # end equation block
                    flags=re.DOTALL|re.MULTILINE|re.UNICODE)
    LABEL_RE = re.compile(r'\\label{(?P<id>.*?)}', flags=re.UNICODE)

    def createToken(self, parent, info, page):
        """Create a LatexBlockEquation token."""
        # TODO: Deprecate this, auto format is needed first so I can update automatically

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ')

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Build the token
        token = LatexBlockEquation(parent, tex=tex, bookmark=eq_id)

        # Add a label
        label = self.LABEL_RE.search(info['equation'])
        if label:
            token.set('label', label.group('id'))
            token.set('tex', token['tex'].replace(str(label.group()), ''))

        return parent

class KatexInlineEquationComponent(components.TokenComponent):
    RE = re.compile(r'(?P<token>\$)(?=\S)(?P<equation>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, parent, info, page):
        """Create LatexInlineEquation"""

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ')

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Create token
        LatexInlineEquation(parent, tex=tex, bookmark=eq_id)
        return parent

class RenderLatexEquation(components.RenderComponent):
    """Render LatexBlockEquation and LatexInlineEquation tokens"""
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use

        if token.name == 'LatexInlineEquation':
            div = html.Tag(parent, 'span', class_='moose-katex-inline-equation',
                           id_=token['bookmark'], **token.attributes)
            display = 'false'

        else:
            # Wrap all equation related items in an outer div
            div = html.Tag(parent, 'span', class_='moose-katex-block-equation')
            display = 'true'

            # Create equation content and number (if it is valid)
            html.Tag(div, 'span', class_='moose-katex-equation table-cell',
                     id_=token['bookmark'],
                     **token.attributes)
            if token['label'] is not None:
                num = html.Tag(div, 'span', class_='moose-katex-equation-number')
                html.String(num, content=u'({})'.format(token['number']))

        # Build the KaTeX script
        script = html.Tag(div, 'script')
        config = dict()
        config['displayMode'] = display
        config['throwOnError'] = 'false'
        if self.extension.macros:
            config['macros'] = self.extension.macros

        config_str = ','.join('{}:{}'.format(key, value) for key, value in config.items())
        content = u'var element = document.getElementById("%s");' % token['bookmark']
        content += u'katex.render("%s", element, {%s});' % \
                   (token['tex'].encode('unicode_escape'), config_str.encode('unicode_escape'))
        html.String(script, content=content)

        return parent

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use
        if token.name == 'LatexInlineEquation':
            latex.String(parent, content=u'${}$'.format(token['tex']), escape=False)
        else:
            cmd = 'equation' if token['number'] else 'equation*'
            env = latex.Environment(parent, cmd)
            if token['label']:
                latex.Command(env, 'label', string=token['label'], end='\n')
            latex.String(env, content=token['tex'], escape=False)

        return parent

class RenderEquationLink(core.RenderShortcutLink):

    def createLatex(self, parent, token, page):
        labels = self.translator.getMetaData(page, 'labels')
        key = token['key']
        if key in labels:
            latex.String(parent, content=self.extension['prefix'] + '~', escape=False)
            latex.Command(parent, 'eqref', string=key)
            return parent
        return core.RenderShortcutLink.createLatex(self, parent, token, page)
