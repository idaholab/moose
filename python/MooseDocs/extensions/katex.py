#pylint: disable=missing-docstring
import re
import uuid

from MooseDocs.base import components
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html, latex
from MooseDocs.tree.base import Property

def make_extension(**kwargs):
    """Create an instance of the Extension object."""
    return KatexExtension(**kwargs)

class LatexBlockEquation(tokens.CountToken):
    r"""
    Token for LaTeX block level equations (e.g., \begin{equation} ... \end{equation}.
    """
    PROPERTIES = [Property('tex', required=True, ptype=str)]

class LatexInlineEquation(LatexBlockEquation):
    """Token for inline equations."""
    pass

class KatexExtension(components.Extension):
    """
    Extension object for parsing and rendering LaTeX equations with KaTeX.
    """
    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        config['prefix'] = ('Eq.', r"The prefix to used when referring to an equation by " \
                                   r"the \\label content.")
        return config

    def extend(self, reader, renderer):
        """
        Add the necessary components for reading and rendering LaTeX.
        """
        reader.addInline(KatexBlockEquationComponent(), location='_begin')
        reader.addInline(KatexInlineEquationComponent(), location='_begin')

        renderer.add(LatexBlockEquation, RenderLatexEquation())
        renderer.add(LatexInlineEquation, RenderLatexEquation())

class KatexBlockEquationComponent(components.TokenComponent):
    """
    Component for reading LaTeX block equations.
    """
    RE = re.compile(r'^\\begin{(?P<cmd>equation\*{0,1})}' # start equation block
                    r'(?P<equation>.*?)'                  # tex equation
                    r'^\\end{(?P=cmd)}',                  # end equation block
                    flags=re.DOTALL|re.MULTILINE|re.UNICODE)
    LABEL_RE = re.compile(r'\\label{(?P<id>.*?)}', flags=re.UNICODE)

    def createToken(self, info, parent):
        """Create a LatexBlockEquation token."""

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ').encode('string-escape')

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Build the token
        is_numbered = not info['cmd'].endswith('*')
        prefix = unicode(self.extension.get('prefix')) if is_numbered else None
        token = LatexBlockEquation(parent, tex=tex, prefix=prefix, id_=eq_id)

        # Add a label
        label = self.LABEL_RE.search(info['equation'])
        if label and not is_numbered:
            msg = "TeX non-numbered equations (e.g., equations*) may not include a \\label, since" \
                  "it will not be possible to refer to the equation."
            raise exceptions.TokenizeException(msg)

        elif label:
            token.tex = token.tex.replace(label.group().encode('ascii'), '') #pylint: disable=attribute-defined-outside-init
            tokens.Shortcut(parent.root, key=label.group('id'),
                            link=u'#{}'.format(eq_id),
                            string=u'{} {}'.format(prefix, token.number))

        return parent

class KatexInlineEquationComponent(components.TokenComponent):
    RE = re.compile(r'(?P<token>\$)(?=\S)(?P<equation>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, info, parent):
        """Create LatexInlineEquation"""

        # Raw LaTeX appropriate for passing to KaTeX render method
        tex = r'{}'.format(info['equation']).strip('\n').replace('\n', ' ').encode('string-escape')

        # Define a unique equation ID for use by KaTeX
        eq_id = 'moose-equation-{}'.format(uuid.uuid4())

        # Create token
        LatexInlineEquation(parent, tex=tex, id_=eq_id)
        return parent

class RenderLatexEquation(components.RenderComponent):
    """Render LatexBlockEquation and LatexInlineEquation tokens"""
    def createHTML(self, token, parent): #pylint: disable=no-self-use

        if isinstance(token, LatexInlineEquation):
            div = html.Tag(parent, 'span', class_='moose-katex-inline-equation', **token.attributes)
            display = 'false'

        else:
            # Wrap all equation related items in an outer div
            div = html.Tag(parent, 'span', class_='moose-katex-block-equation')
            display = 'true'

            # Create equation content and number (if it is valid)
            html.Tag(div, 'span', class_='moose-katex-equation table-cell', **token.attributes)
            if token.number is not None:
                num = html.Tag(div, 'span', class_='moose-katex-equation-number')
                html.String(num, content=u'({})'.format(token.number))

        # Build the KaTeX script
        script = html.Tag(div, 'script')
        content = u'var element = document.getElementById("%s");' % token['id']
        content += u'katex.render("%s", element, {displayMode:%s,throwOnError:false});' % \
                   (token.tex, display)
        html.String(script, content=content)

        return parent

    def createLatex(self, token, parent): #pylint: disable=no-self-use
        if isinstance(token, LatexInlineEquation):
            latex.String(parent, content=u'${}$'.format(token.tex))
        else:
            cmd = 'equation' if token.number else 'equation*'
            latex.Environment(parent, cmd, string=unicode(token.tex))
        return parent
