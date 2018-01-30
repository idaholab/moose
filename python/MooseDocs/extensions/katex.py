#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import re
import uuid
import logging

from markdown.inlinepatterns import Pattern
from markdown.util import etree

from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

LOG = logging.getLogger(__file__)

class KatexExtension(MooseMarkdownExtension):
    """
    Adds KaTeX support for MooseDocs markdown.
    """
    @staticmethod
    def defaultConfig():
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Add support for KaTeX math.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        md.inlinePatterns.add('katex-inline',
                              KatexInline(markdown_instance=md, **config),
                              '>backtick')

        md.inlinePatterns.add('katex-equation',
                              KatexEquation(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('katex-star-equation',
                              KatexStarEquation(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('katex-double-dollar-equation',
                              KatexDoubleDollarEquation(markdown_instance=md, **config),
                              '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create KatexExtension"""
    return KatexExtension(*args, **kwargs)

class KatexBase(MooseMarkdownCommon, Pattern):
    """
    Base class for Pattern extensions.
    """
    RE = None
    DISPLAY_MODE = 'true'

    @staticmethod
    def defaultSettings():
        settings = MooseMarkdownCommon.defaultSettings()
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

    def equationID(self, tex): #pylint: disable=unused-argument, no-self-use
        """
        Return the equation id.
        """
        return 'moose-katex-equation-{}'.format(uuid.uuid4()), tex

    def equationDiv(self, eq_id): #pylint: disable=unused-argument
        """
        Return the div object where the equation will be displayed.
        """
        raise NotImplementedError("The equationDiv method is abstract and must be overridden.")

    def handleMatch(self, match):
        """
        Create KaTeX code.
        """
        try:
            tex = r'{}'.format(match.group('tex')).replace('\n', ' ').encode('string-escape')
        except UnicodeEncodeError:
            msg = "Unable to escape latex in {}.".format(self.markdown.current.filename)
            return self.createErrorElement(msg)

        eq_id, tex = self.equationID(tex)
        tag = self.equationDiv(eq_id)
        self.buildScript(tag, eq_id, tex)
        return tag

    def buildScript(self, parent, eq_id, tex):
        """
        Create the KaTeX script element.
        """
        script = etree.SubElement(parent, 'script')
        text = 'var element = document.getElementById("{}");'.format(eq_id)
        text += 'try{katex.render("%s", element, {displayMode:%s});}' % (tex, self.DISPLAY_MODE)
        text += 'catch (exception){'
        text += 'console.log("KaTeX render failed: {}");'.format(tex)
        text += 'var err=document.createElement("span");'
        text += 'err.setAttribute("class", "moose-katex-error");'
        text += 'err.textContent = "LaTeX Error: {}";'.format(tex)
        text += 'element.appendChild(err);'
        text += '}'
        script.text = self.markdown.htmlStash.store(text)

class KatexInline(KatexBase):
    """
    Inline math support with KaTex.
    """
    RE = r'\$(?P<tex>.*?)\$'
    DISPLAY_MODE = 'false'

    def equationDiv(self, eq_id):
        span = etree.Element('span')
        span.set('class', 'moose-katex-inline')
        span.set('id', eq_id)
        return span

class KatexEquation(KatexBase):
    """
    Block math support with KaTeX.
    """
    RE = r'\\begin{equation}(?P<tex>.*?)\\end{equation}'

    def equationID(self, tex):
        """
        Determine the equation ID.
        """

        self.markdown.EQUATION_COUNT += 1
        label = re.search(r'\\\\label{(?P<label>.*?)}', tex)
        if label:
            eq_id = 'moose-katex-equation-{}'.format(label.group('label')).replace(':', '-')
            tex = tex[:label.start()] + tex[label.end():] # KaTeX can't parse the \label
        else:
            eq_id = 'moose-katex-equation-{}'.format(self.markdown.EQUATION_COUNT)

        return eq_id, tex

    def equationDiv(self, eq_id):
        """
        Create tags for numbered equations.
        """
        div = etree.Element('div')
        div.set('class', 'moose-katex-block')

        eqn = etree.SubElement(div, 'div')
        eqn.set('class', 'moose-katex-equation')
        eqn.set('id', eq_id)
        eqn.set('data-moose-katex-equation-number', str(self.markdown.EQUATION_COUNT))

        num = etree.SubElement(div, 'div')
        num.set('class', 'moose-katex-block-number')
        num.text = '({})'.format(self.markdown.EQUATION_COUNT)

        return div

class KatexStarEquation(KatexBase):
    """
    Block math support with KaTeX.
    """
    RE = r'\\begin{equation\*}(?P<tex>.*?)\\end{equation\*}'

    def equationDiv(self, eq_id):
        """
        Create equation container tag.
        """
        eqn = etree.Element('div')
        eqn.set('class', 'moose-katex-equation')
        eqn.set('id', eq_id)
        return eqn

class KatexDoubleDollarEquation(KatexStarEquation):
    """
    Block math support with KaTex.
    """
    RE = r'\$\$(?P<tex>.*?)\$\$'
