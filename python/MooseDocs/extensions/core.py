#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Defines the "core" extension for translating MooseDocs markdown to HTML and LaTeX.
"""
import re
import uuid
import logging

import anytree

from MooseDocs.base import components, renderers, Translator
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """
    Create and return the CoreExtension object for converting from markdown to html/latex.
    """
    return CoreExtension(**kwargs)


Section = tokens.newToken(u'Section')
Word = tokens.newToken(u'Word', content=u'')
Space = tokens.newToken(u'Space', count=1)
Break = tokens.newToken(u'Break', count=1)
Punctuation = tokens.newToken(u'Punctuation', content=u'')
Number = tokens.newToken(u'Number', content=u'')
Code = tokens.newToken(u'Code', content=u'', language=u'text', escape=True)
Heading = tokens.newToken(u'Heading', level=1)
Paragraph = tokens.newToken(u'Paragraph')
OrderedList = tokens.newToken(u'OrderedList', browser_default=True, start=1)
UnorderedList = tokens.newToken(u'UnorderedList', browser_default=True)
ListItem = tokens.newToken(u'ListItem')
Link = tokens.newToken(u'Link', url=u'', tooltip=True)
Shortcut = tokens.newToken(u'Shortcut', key=u'', link=u'')
ShortcutLink = tokens.newToken(u'ShortcutLink', key=u'')
Monospace = tokens.newToken(u'Monospace', content=u'')
Strong = tokens.newToken(u'Strong')
Emphasis = tokens.newToken(u'Emphasis')
Underline = tokens.newToken(u'Underline')
Strikethrough = tokens.newToken(u'Strikethrough')
Quote = tokens.newToken(u'Quote')
Superscript = tokens.newToken(u'Superscript')
Subscript = tokens.newToken(u'Subscript')
Label = tokens.newToken(u'Label', text=u'')

class CoreExtension(components.Extension):
    """
    The core markdown extension object. Extension objects add the tokenize and rendering components.
    """
    @staticmethod
    def defaultConfig():
        """CoreExtension configuration options."""
        config = components.Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        """
        Add the extension components.
        """

        # Block tokenize components
        reader.addBlock(CodeBlock())
        reader.addBlock(QuoteBlock())
        reader.addBlock(HeadingBlock())
        reader.addBlock(OrderedListBlock())
        reader.addBlock(UnorderedListBlock())
        reader.addBlock(ShortcutBlock())
        reader.addBlock(ParagraphBlock())
        reader.addBlock(EndOfFileBlock())

        # Inline tokenize components
        reader.addInline(FormatInline())
        reader.addInline(LinkInline())
        reader.addInline(ShortcutLinkInline())
        reader.addInline(PunctuationInline())
        reader.addInline(NumberInline())
        reader.addInline(WordInline())
        reader.addInline(BreakInline())
        reader.addInline(SpaceInline())

        # Render components
        renderer.add(u'Heading', RenderHeading())
        renderer.add(u'Code', RenderCode())
        renderer.add(u'ShortcutLink', RenderShortcutLink())
        renderer.add(u'Shortcut', RenderShortcut())
        renderer.add(u'Monospace', RenderMonospace())
        renderer.add(u'Break', RenderBreak())
        renderer.add(u'ErrorToken', RenderError())

        renderer.add(u'Link', RenderLink())
        renderer.add(u'Paragraph', RenderParagraph())
        renderer.add(u'UnorderedList', RenderUnorderedList())
        renderer.add(u'OrderedList', RenderOrderedList())
        renderer.add(u'ListItem', RenderListItem())
        renderer.add(u'Quote', RenderQuote())
        renderer.add(u'Strong', RenderStrong())
        renderer.add(u'Underline', RenderUnderline())
        renderer.add(u'Emphasis', RenderEmphasis())
        renderer.add(u'Strikethrough', RenderStrikethrough())
        renderer.add(u'Superscript', RenderSuperscript())
        renderer.add(u'Subscript', RenderSubscript())
        renderer.add(u'Label', RenderLabel())
        renderer.add(u'Punctuation', RenderPunctuation())
        renderer.add(u'DisabledToken', RenderDisabled())
        renderer.add(u'Space', RenderSpace())

        renderer.add(u'Word', RenderString())
        renderer.add(u'Number', RenderString())
        renderer.add(u'String', RenderString())

        #TODO: Make a generic preamble method?
        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage(u'amsmath')
            renderer.addPackage(u'soul')
            renderer.addPackage(u'hyperref')

# Documenting all these classes is far to repetitive and useless.
#pylint: disable=missing-docstring

class CodeBlock(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^'         # start of string or empty line
                    r'`{3}(?P<settings>.*?)$' # start of code with key=value settings
                    r'(?P<code>.*?)^`{3}'     # code and end of fenced block
                    r'(?=\n*\Z|\n{2,})',         # end of string or empty line
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    @staticmethod
    def defaultSettings():
        settings = components.TokenComponent.defaultSettings()
        settings['language'] = (u'text', "The code language to use for highlighting.")
        return settings

    def createToken(self, parent, info, page):
        args = info['settings'].split()
        if args and ('=' not in args[0]):
            self.settings['language'] = args[0]

        return Code(parent, content=info['code'], language=self.settings['language'],
                    **self.attributes)

class QuoteBlock(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})'         # start of string or empty line
                    r'(?P<quote>^>[ $].*?)'  # quote content
                    r'(?=\n*\Z|\n{2,})',        # end of string or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page):
        content = []
        for line in info['quote'].rstrip('\n').split('\n'):
            if line == u'>':
                content.append('')
            elif line.startswith(u'> '):
                content.append(line[2:])
            else:
                raise Exception(repr(line))

        #TODO: error check that all lines begin with '> '
        quote = Quote(parent)
        self.reader.tokenize(quote, '\n'.join(content), page, line=info.line)
        return quote

class HeadingBlock(components.TokenComponent):
    """
    Hash style markdown headings with settings.

    # Heading Level One with=settings
    """
    TOKEN = Heading
    RE = re.compile(r'(?:\A|\n{2,})'             # start of string or empty line
                    r'^(?P<level>#{1,6}) '       # match 1 to 6 #'s at the beginning of line
                    r'(?P<inline>.*?) *'         # heading text that will be inline parsed
                    r'(?P<settings>\s+\w+=.*?)?' # optional match key, value settings
                    r'(?=\n*\Z|\n{2,})',         # match up to end of string or newline(s)
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page):
        content = info['inline']
        heading = Heading(parent, level=info['level'].count('#'), **self.attributes)
        Label(heading, text=content)
        return heading

class ListBlock(components.TokenComponent):
    RE = None
    ITEM_RE = None
    TOKEN = None

    def createToken(self, parent, info, page):
        marker = info['marker']
        n = len(marker)
        token = tokens.Token(self.TOKEN, parent) #pylint: disable=not-callable
        strip_regex = re.compile(r'^ {%s}(.*?)$' % n, flags=re.MULTILINE)

        for item in self.ITEM_RE.finditer(info['items']):
            content = ' '*n + item.group('item')
            indent = re.search(r'^\S', content, flags=re.MULTILINE|re.UNICODE)
            if indent:
                msg = "List item content must be indented by {} to match the list item " \
                       "characters of '{}', to end a list item you must use two empty lines."
                raise exceptions.MooseDocsException(msg, n, marker)

            content = strip_regex.sub(r'\1', content)
            self.reader.tokenize(ListItem(token), content, page, line=info.line)

        return token

class UnorderedListBlock(ListBlock):
    RE = re.compile(r'(?:\A|\n{2,})'                   # start of string or empty line
                    r'(?P<items>(?P<marker>^- ).*?)'   # all items
                    r'(?=\n{3,}|\n*\Z|\n{2}^[^-\s])',  # stop with 2 empty or 1 not with marker
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    ITEM_RE = re.compile(r'^- (?P<item>.*?)(?=\Z|^- )', flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    TOKEN = u'UnorderedList'

class OrderedListBlock(ListBlock):
    """Ordered lists."""
    RE = re.compile(r'(?:\A|\n{2,})'                        # start of string or empty line
                    r'(?P<items>(?P<marker>^[0-9]+\. ).*?)' # all items
                    r'(?=\n{3,}|\n*\Z|\n{2}^[^[0-9\s])',    # stop with 2 empty or 1 not with marker
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    ITEM_RE = re.compile(r'^[0-9]+\. (?P<item>.*?)(?=\Z|^[0-9]+\. )',
                         flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    TOKEN = u'OrderedList'

    #TODO: figure out how to handle settings???
    #TODO: combine regex for ordered/unordered
    # 1. ::start=42 type=a:: This is the actual content.???

    @staticmethod
    def defaultSettings():
        settings = ListBlock.defaultSettings()
        settings['type'] = ('1', "The list type (1, A, a, i, or I).")
        return settings

    def createToken(self, parent, info, page):
        token = ListBlock.createToken(self, parent, info, page)
        token['start'] = int(info['marker'].strip('. '))
        return token

class ShortcutBlock(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^\[(?P<key>\w+)\]: ' # shortcut key
                    r'(?P<link>.*?)'                    # shortcut link
                    r'(?=\Z|\n{2,})',                   # stop new line or end of file
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page):
        return Shortcut(parent, key=info['key'], link=info['link'], string=info['key'])

class ParagraphBlock(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})'   # start of string of empty line
                    r'(?P<inline>.*?)' # content
                    r'(?=\Z|\n{2,})',  # stop with end or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page): #pylint: disable=unused-argument
        return Paragraph(parent)

class EndOfFileBlock(components.TokenComponent):
    RE = re.compile(r'.*', flags=re.UNICODE|re.MULTILINE|re.DOTALL)
    def createToken(self, parent, info, page):
        return parent

class LinkInline(components.TokenComponent):
    """
    Links are defined as: [link text](link address).

    The regex is a bit tricky for this when the line also contains a "shortcut link", as follows:

       [shortcut link] and regular [link text](link address).

    Without the negative lookahead after the first "[" the match would capture the beginning at
    the shortcut link.
    """

    RE = re.compile(r'\[(?!\w+?\] )'                # start of link, see note above
                    r'(?P<inline>.*?)\]'            # link text and end of text
                    r'\((?P<url>.*?)'               # link url
                    r'(?:\s+(?P<settings>.*?))?\)', # settings
                    flags=re.UNICODE)

    def createToken(self, parent, info, page):
        return Link(parent, url=info['url'], **self.attributes)

class ShortcutLinkInline(components.TokenComponent):
    RE = re.compile(r'\['                        # opening [
                    r'(?P<key>.+?)'              # key
                    r'(?:\s+(?P<settings>.*?))?' # settings
                    r'\]',                       # closing ]
                    flags=re.UNICODE)
    def createToken(self, parent, info, page):
        ShortcutLink(parent, key=info['key'], **self.attributes)
        return parent

class BreakInline(components.TokenComponent):
    RE = re.compile(r'(?P<break>\n+)')
    def createToken(self, parent, info, page):
        Break(parent, count=len(info['break']))
        return parent

class SpaceInline(components.TokenComponent):
    RE = re.compile(r'(?P<space> +)')
    def createToken(self, parent, info, page):
        Space(parent, count=len(info['space']))
        return parent

class PunctuationInline(components.TokenComponent):
    RE = re.compile(r'(([^A-Za-z0-9\s])\2*)')
    def createToken(self, parent, info, page):
        Punctuation(parent, content=info[0])
        return parent

class NumberInline(components.TokenComponent):
    RE = re.compile(r'([0-9]+)')
    def createToken(self, parent, info, page):
        Number(parent, content=info[0])
        return parent

class WordInline(components.TokenComponent):
    RE = re.compile(r'([A-Za-z]+)')
    def createToken(self, parent, info, page):
        Word(parent, content=info[0])
        return parent

class FormatInline(components.TokenComponent):
    RE = re.compile(r'(?P<token>[\@|\^|\=|\*|\+|~`])(?=\S)(?P<inline>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, parent, info, page):
        tok = info['token']

        # Sub/super script must have word before the rest cannot
        if (tok == '^') or (tok == '@'):
            if not parent.children or (not parent.children[-1].name == u'Word'):
                return None
        elif parent.children and (parent.children[-1].name == u'Word'):
            return None

        if tok == u'@':
            return Subscript(parent)
        elif tok == u'^':
            return Superscript(parent)
        elif tok == u'=':
            return Underline(parent)
        elif tok == u'*':
            return Emphasis(parent)
        elif tok == u'+':
            return Strong(parent)
        elif tok == u'~':
            return Strikethrough(parent)
        elif tok == u'`':
            return Monospace(parent, content=info['inline'], recursive=False)

####################################################################################################
# Rendering.
####################################################################################################

class RenderHeading(components.RenderComponent):
    LATEX_SECTIONS = ['part', 'chapter', 'section', 'subsection', 'subsubsection', 'paragraph',
                      'subparagraph']

    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        if not token.get('id'):
            token.set('id', token.text(u'-').lower())
        return html.Tag(parent, 'h{}'.format(token['level']), token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, self.LATEX_SECTIONS[token['level']], start='\n')

class RenderLabel(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        pass

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use
        label = token.get('id', re.sub(r' +', r'-', token['text'].lower()))
        return latex.Command(parent, 'label', string=label)

class RenderCode(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        language = 'language-{}'.format(token['language'])
        pre = html.Tag(parent, 'pre', token)
        code = html.Tag(pre, 'code', class_=language)
        html.String(code, content=token['content'], escape=token['escape'])
        return pre

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'verbatim', string=token['content'], after_begin='',
                                 before_end='')

class RenderShortcutLink(components.RenderComponent):
    def __init__(self, *args, **kwargs):
        components.RenderComponent.__init__(self, *args, **kwargs)
        self.__cache = dict()

    def createHTML(self, parent, token, page):
        a = html.Tag(parent, 'a', token)
        node = self.getShortcut(token)
        a['href'] = node['link']
        for child in node.children:
            self.renderer.render(a, child, page)
        return a

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        tag.addClass('tooltipped')
        tag['data-tooltip'] = tag['href']
        tag['data-position'] = 'top'
        return tag

    def createLatex(self, parent, token, page):
        cmd = latex.CustomCommand(parent, 'href')
        node = self.getShortcut(token)

        latex.Brace(cmd, string=node['link'])
        return latex.Brace(cmd)

    def getShortcut(self, token):
        key = token['key']
        if key in self.__cache:
            return self.__cache[key]

        #TODO: error if more than one found
        for node in anytree.PreOrderIter(token.root, maxlevel=None):
            if node.name == 'Shortcut' and node['key'] == key:
                with Translator.LOCK:
                    self.__cache[key] = node
                return node

        msg = "The shortcut link key '{}' was not located in the list of shortcuts."
        raise exceptions.MooseDocsException(msg, key)

class RenderShortcut(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createLatex(self, *args):
        pass

class RenderMonospace(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        code = html.Tag(parent, 'code')
        html.String(code, content=token['content'], escape=True)
        return code

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use
        code = latex.Command(parent, 'texttt')
        latex.String(code, content=token['content'])
        return

class RenderBreak(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return html.String(parent, content=u' ')

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.String(parent, content=u' ')

class RenderLink(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'a', token, href=token['url'])

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        if token['tooltip']:
            tag.addClass('tooltipped')
            if 'data-tooltip' not in tag:
                tag['data-tooltip'] = tag['href']
            tag['data-position'] = 'top'
        return tag

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        url = token['url'].lstrip('#')
        cmd = latex.CustomCommand(parent, 'href')
        latex.Brace(cmd, string=url)
        arg1 = latex.Brace(cmd)
        return arg1

class RenderParagraph(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return html.Tag(parent, 'p', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        latex.CustomCommand(parent, 'par', start='\n', end=' ')
        return parent

class RenderOrderedList(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return html.Tag(parent, 'ol', token)

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use
        tag = self.createHTML(parent, token, page)
        if token.get('browser_default', True):
            tag.addClass('browser-default')
        tag['start'] = token['start']
        return tag

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'enumerate')

class RenderUnorderedList(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'ul', token)

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use
        tag = self.createHTML(parent, token, page)
        # TODO: accessing token['browser_default'] cause problems, I can't figure out why
        if token.get('browser_default', True):
            tag.addClass('browser-default')
        return tag

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'itemize')

class RenderListItem(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'li', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        latex.Command(parent, 'item')
        return parent

class RenderString(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.String(parent, content=token['content'])

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use
        return latex.String(parent, content=token['content'])

class RenderSpace(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.String(parent, content=u' '*token['count'])

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use
        return latex.String(parent, content=u' '*token['count'])

class RenderQuote(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'blockquote', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'quote', after_begin='')

class RenderStrong(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'strong', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textbf')

class RenderEmphasis(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'em', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'emph')

class RenderUnderline(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'u', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'ul')

class RenderStrikethrough(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'strike', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument

        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'so')

class RenderSuperscript(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'sup', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textsuperscript')

class RenderSubscript(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'sub', token)

    def createLatex(self, parent, token, page): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textsubscript')

class RenderPunctuation(RenderString):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        content = token['content']
        if content == u'--':
            return html.String(parent, content=u'&ndash;')
        elif content == u'---':
            return html.String(parent, content=u'&mdash;')

        return RenderString.createHTML(self, parent, token, page)

class RenderError(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', token)
        div.addClass("moose-exception")
        html.String(div, content=token.info[0])
        return div

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use

        id_ = uuid.uuid4()
        a = html.Tag(parent, 'a', class_="moose-exception modal-trigger", href='#{}'.format(id_))
        html.String(a, content=token.info[0])

        modal = html.Tag(parent.root, 'div', id_=id_, class_="modal")
        content = html.Tag(modal, 'div', class_="modal-content")
        head = html.Tag(content, 'h2')
        html.String(head, content=u'Tokenize Error')
        p = html.Tag(content, 'p')

        html.String(p, content=unicode(token['message']))
        html.Tag(p, 'br', close=False)
        html.String(p, content=u'{}:{}'.format(page.local, token.info.line))
        html.Tag(p, 'br', close=False)

        pre = html.Tag(content, 'pre')
        code = html.Tag(pre, 'code', class_="language-markdown")
        html.String(code, content=token.info[0], escape=True)

        footer = html.Tag(modal, 'div', class_="modal-footer grey lighten-3")
        done = html.Tag(footer, 'a', class_="modal-action modal-close btn-flat")
        html.String(done, content=u"Done")

        trace = token.get('traceback', None)
        if trace is not None:
            pre = html.Tag(content, 'pre', style="font-size:80%;")
            html.String(pre, content=trace, escape=True)

        return content

    def createLatex(self, parent, token, page):
        pass

class RenderDisabled(components.RenderComponent):

    def createLatex(self, parent, token, page):
        pass

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'p', class_='moose-disabled')
