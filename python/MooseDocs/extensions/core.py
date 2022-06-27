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

import moosetree

from ..base import components, renderers, Executioner, Extension
from ..common import exceptions
from ..tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """
    Create and return the CoreExtension object for converting from markdown to html/latex.
    """
    return CoreExtension(**kwargs)


Section = tokens.newToken('Section')

Word = tokens.newToken('Word', content='')
Space = tokens.newToken('Space', count=1)
Break = tokens.newToken('Break', count=1)
Punctuation = tokens.newToken('Punctuation', content='')
Number = tokens.newToken('Number', content='')
Code = tokens.newToken('Code', content='', language='text', escape=True)
Heading = tokens.newToken('Heading', level=1)
Paragraph = tokens.newToken('Paragraph')
OrderedList = tokens.newToken('OrderedList', browser_default=True, start=1)
UnorderedList = tokens.newToken('UnorderedList', browser_default=True)
ListItem = tokens.newToken('ListItem')
Link = tokens.newToken('Link', url='')
Shortcut = tokens.newToken('Shortcut', key='', link='', prefix='')
ShortcutLink = tokens.newToken('ShortcutLink', key='')
Monospace = tokens.newToken('Monospace', content='')
Strong = tokens.newToken('Strong')
Emphasis = tokens.newToken('Emphasis')
Underline = tokens.newToken('Underline')
Strikethrough = tokens.newToken('Strikethrough')
Quote = tokens.newToken('Quote')
Superscript = tokens.newToken('Superscript')
Subscript = tokens.newToken('Subscript')
LineBreak = tokens.newToken('LineBreak')

class CoreExtension(Extension):
    """
    The core markdown extension object. Extension objects add the tokenize and rendering components.
    """
    @staticmethod
    def defaultConfig():
        """CoreExtension configuration options."""
        config = Extension.defaultConfig()
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
        reader.addInline(EscapeCharacter())
        reader.addInline(FormatInline())
        reader.addInline(LinkInline())
        reader.addInline(ShortcutLinkInline())
        reader.addInline(LineBreakInline())
        reader.addInline(PunctuationInline())
        reader.addInline(NumberInline())
        reader.addInline(WordInline())
        reader.addInline(BreakInline())
        reader.addInline(SpaceInline())

        # Render components
        renderer.add('Heading', RenderHeading())
        renderer.add('Code', RenderCode())
        renderer.add('ShortcutLink', RenderShortcutLink())
        renderer.add('Shortcut', RenderShortcut())
        renderer.add('Monospace', RenderMonospace())
        renderer.add('Break', RenderBreak())
        renderer.add('LineBreak', RenderLineBreak())
        renderer.add('ErrorToken', RenderError())

        renderer.add('Link', RenderLink())
        renderer.add('Paragraph', RenderParagraph())
        renderer.add('UnorderedList', RenderUnorderedList())
        renderer.add('OrderedList', RenderOrderedList())
        renderer.add('ListItem', RenderListItem())
        renderer.add('Quote', RenderQuote())
        renderer.add('Strong', RenderStrong())
        renderer.add('Underline', RenderUnderline())
        renderer.add('Emphasis', RenderEmphasis())
        renderer.add('Strikethrough', RenderStrikethrough())
        renderer.add('Superscript', RenderSuperscript())
        renderer.add('Subscript', RenderSubscript())
        renderer.add('Punctuation', RenderPunctuation())
        renderer.add('DisabledToken', RenderDisabled())
        renderer.add('Space', RenderSpace())

        renderer.add('Word', RenderString())
        renderer.add('Number', RenderString())
        renderer.add('String', RenderString())

        #TODO: Make a generic preamble method?
        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage('amsmath')
            renderer.addPackage('soul')
            renderer.addPackage('hyperref',
                                linkcolor='blue',
                                citecolor='blue',
                                filecolor='blue',
                                urlcolor='blue',
                                colorlinks='true')

class CodeBlock(components.ReaderComponent):
    RE = re.compile(r'(?:\A|\n{2,})^'         # start of string or empty line
                    r'`{3}(?P<settings>.*?)$' # start of code with key=value settings
                    r'(?P<code>.*?)^`{3}'     # code and end of fenced block
                    r'(?=\n*\Z|\n{2,})',      # end of string or empty line
                    flags=re.UNICODE|re.MULTILINE|re.DOTALL)

    @staticmethod
    def defaultSettings():
        settings = components.ReaderComponent.defaultSettings()
        settings['language'] = ('text', "The code language to use for highlighting.")
        return settings

    def createToken(self, parent, info, page, settings):
        args = info['settings'].split()
        if args and ('=' not in args[0]):
            settings['language'] = args[0]

        return Code(parent, content=info['code'], language=settings['language'],
                    **self.attributes(settings))

class QuoteBlock(components.ReaderComponent):
    RE = re.compile(r'(?:\A|\n{2,})'         # start of string or empty line
                    r'(?P<quote>^>[ $].*?)'  # quote content
                    r'(?=\n*\Z|\n{2,})',        # end of string or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page, settings):
        content = []
        for line in info['quote'].rstrip('\n').split('\n'):
            if line == '>':
                content.append('')
            elif line.startswith('> '):
                content.append(line[2:])
            else:
                raise Exception(repr(line))

        #TODO: error check that all lines begin with '> '
        quote = Quote(parent)
        self.reader.tokenize(quote, '\n'.join(content), page, line=info.line)
        return quote

class HeadingBlock(components.ReaderComponent):
    """
    Hash style markdown headings with settings.

    # Heading Level One with=settings

    The regex reads inline commands, such as autolinks, as part of the header text, e.g.,

        ## [Home](index.md exact=True) id=home-link

    The trick is that header settings need to be distinguished from inline command settings. This is
    accomplished by doing a negative lookahead for a "]" or ")" following the 'key=value' match. It
    should be safe to say that inputs for header settings (currently, 'style', 'class', or 'id') can
    never contain a "]" or ")", since HTML classes and CSS grammar do not allow them in names (see
    https://www.w3.org/TR/CSS21/grammar.html#scanner).
    """
    TOKEN = Heading
    RE = re.compile(r'(?:\A|\n{2,})'            # start of string or empty line
                    r'^(?P<level>#{1,6}) '      # hashes indicating header level
                    r'(?P<inline>.*?) *'        # heading text to be inline parsed
                    r'(?!\w+=[^\n]*?(?:\]|\)))' # inline command settings lookahead
                    r'(?P<settings>\w+=.*?)?'   # optional 'key=value' settings
                    r'(?=\n*\Z|\n{2,})',        # end of string or newline(s)
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page, settings):
        heading = Heading(parent, level=info['level'].count('#'), **self.attributes(settings))
        return heading

class ListBlock(components.ReaderComponent):
    RE = None
    ITEM_RE = None
    TOKEN = None

    def createToken(self, parent, info, page, settings):
        marker = info['marker']
        n = len(marker)
        token = tokens.Token(self.TOKEN, parent)
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
    TOKEN = 'UnorderedList'

class OrderedListBlock(ListBlock):
    """Ordered lists."""
    RE = re.compile(r'(?:\A|\n{2,})'                        # start of string or empty line
                    r'(?P<items>(?P<marker>^[0-9]+\. ).*?)' # all items
                    r'(?=\n{3,}|\n*\Z|\n{2}^[^[0-9\s])',    # stop with 2 empty or 1 not with marker
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    ITEM_RE = re.compile(r'^[0-9]+\. (?P<item>.*?)(?=\Z|^[0-9]+\. )',
                         flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    TOKEN = 'OrderedList'

    #TODO: figure out how to handle settings???
    #TODO: combine regex for ordered/unordered
    # 1. ::start=42 type=a:: This is the actual content.???

    @staticmethod
    def defaultSettings():
        settings = ListBlock.defaultSettings()
        settings['type'] = ('1', "The list type (1, A, a, i, or I).")
        return settings

    def createToken(self, parent, info, page, settings):
        token = ListBlock.createToken(self, parent, info, page, settings)
        token['start'] = int(info['marker'].strip('. '))
        return token

class ShortcutBlock(components.ReaderComponent):
    RE = re.compile(r'(?:\A|\n{2,})^\[(?P<key>\w+)\]: ' # shortcut key
                    r'(?P<link>.*?)'                    # shortcut link
                    r'(?=\Z|\n{2,})',                   # stop new line or end of file
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page, settings):
        return Shortcut(parent, key=info['key'], link=info['link'], string=info['key'])

class ParagraphBlock(components.ReaderComponent):
    RE = re.compile(r'(?:\A|\n{2,})'   # start of string of empty line
                    r'(?P<inline>.*?)' # content
                    r'(?=\Z|\n{2,})',  # stop with end or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, parent, info, page, settings):
        return Paragraph(parent)

class EndOfFileBlock(components.ReaderComponent):
    RE = re.compile(r'.*', flags=re.UNICODE|re.MULTILINE|re.DOTALL)
    def createToken(self, parent, info, page, settings):
        return parent

class LinkInline(components.ReaderComponent):
    """
    Links are defined as: [link text](link address).

    The regex is a bit tricky for this when the line also contains a "shortcut link", as follows:

       [shortcut] and regular [link text](link address).

    Without the negative lookahead after the first "[" the match would capture the beginning at
    the shortcut link.

    https://regex101.com/r/LXjbHt/6
    """
    RE = re.compile(r'\[(?!\S+?\][ .,])'            # start of link, see note above
                    r'(?P<inline>.*?)\]'            # link text
                    r'\((?P<url>.*?)'               # link url
                    r'(?:\s+(?P<settings>.*?))?\)', # settings
                    flags=re.UNICODE)

    def createToken(self, parent, info, page, settings):
        return Link(parent, url=info['url'], **self.attributes(settings))

class ShortcutLinkInline(components.ReaderComponent):
    """https://regex101.com/r/JLAaBU/1"""
    RE = re.compile(r'\['                        # opening [
                    r'(?P<key>\S+?)'             # key (anything but space)
                    r'(?:\s+(?P<settings>.*?))?' # settings
                    r'\]',                       # closing ]
                    flags=re.UNICODE)
    def createToken(self, parent, info, page, settings):
        ShortcutLink(parent, key=info['key'], **self.attributes(settings))
        return parent

class BreakInline(components.ReaderComponent):
    RE = re.compile(r'(?P<break>\n+)')
    def createToken(self, parent, info, page, settings):
        Break(parent, count=len(info['break']))
        return parent

class LineBreakInline(components.ReaderComponent):
    RE = re.compile(r'(?P<break>\\{2}[\s+$])', flags=re.MULTILINE)
    def createToken(self, parent, info, page, settings):
        LineBreak(parent)
        return parent

class SpaceInline(components.ReaderComponent):
    RE = re.compile(r'(?P<space> +)')
    def createToken(self, parent, info, page, settings):
        Space(parent, count=len(info['space']))
        return parent

class PunctuationInline(components.ReaderComponent):
    RE = re.compile(r'(([^A-Za-z0-9\s])\2*)')
    def createToken(self, parent, info, page, settings):
        Punctuation(parent, content=info[0])
        return parent

class NumberInline(components.ReaderComponent):
    RE = re.compile(r'([0-9]+)')
    def createToken(self, parent, info, page, settings):
        Number(parent, content=info[0])
        return parent

class WordInline(components.ReaderComponent):
    RE = re.compile(r'([A-Za-z]+)')
    def createToken(self, parent, info, page, settings):
        Word(parent, content=info[0])
        return parent

class FormatInline(components.ReaderComponent):
    RE = re.compile(r'(?P<token>[\@|\^|\=|\*|\+|~`])(?=\S)(?P<inline>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, parent, info, page, settings):
        tok = info['token']

        # Sub/super script must have word before the rest cannot
        if (tok == '^') or (tok == '@'):
            if not parent.children or (parent.children[-1].name not in ('Word', 'Number')):
                return None
        elif parent.children and (parent.children[-1].name in ('Word', 'Number')):
            return None

        if tok == '@':
            return Subscript(parent)
        elif tok == '^':
            return Superscript(parent)
        elif tok == '=':
            return Underline(parent)
        elif tok == '*':
            return Emphasis(parent)
        elif tok == '+':
            return Strong(parent)
        elif tok == '~':
            return Strikethrough(parent)
        elif tok == '`':
            return Monospace(parent, content=info['inline'], recursive=False)

class EscapeCharacter(components.ReaderComponent):
    RE = re.compile(r'\\(?P<char>\[|\]|!|\@|\^|\=|\*|\+|~|-)',
                    flags=re.MULTILINE|re.DOTALL)

    def createToken(self, parent, info, page, settings):
        Punctuation(parent, content=info['char'])
        return parent

####################################################################################################
# Rendering.
####################################################################################################

class RenderHeading(components.RenderComponent):
    LATEX_SECTIONS = ['part', 'chapter', 'section', 'subsection', 'subsubsection', 'paragraph',
                      'subparagraph']

    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'h{}'.format(token['level']), token)

    def createLatex(self, parent, token, page):
        sec = latex.Command(parent,
                            self.LATEX_SECTIONS[token['level'] - 1],
                            start='\n',
                            info=token.info)
        id_ = token.get('id')
        if id_:
            latex.Command(sec, 'label', string=id_, escape=False)
        return sec

class RenderCode(components.RenderComponent):
    def createHTML(self, parent, token, page):
        language = 'language-{}'.format(token['language'])

        pre = html.Tag(parent, 'pre', token)
        pre.addClass('moose-pre')
        code = html.Tag(pre, 'code', class_=language)
        html.String(code, content=token['content'], escape=token['escape'])
        return pre

    def createLatex(self, parent, token, page):

        return latex.Environment(parent, 'verbatim',
                                 string=token['content'].strip('\n'),
                                 after_begin='\n',
                                 before_end='\n',
                                 escape=False,
                                 info=token.info)

class RenderShortcutLink(components.RenderComponent):
    def createHTML(self, parent, token, page):
        node = self._getShortcut(page, token['key'])
        a = html.Tag(parent, 'a', token, href=node['link'])
        for child in node.children:
            self.renderer.render(a, child, page)
        return a

    def createLatex(self, parent, token, page):
        node = self._getShortcut(page, token['key'])
        link = node['link'].lstrip('#')
        if len(node) == 0:
            latex.String(parent, content='{}~'.format(node['prefix']), escape=False)
            h = latex.Command(parent, 'ref',
                              string=link,
                              info=token.info,
                              escape=False)
        else:
            h = latex.Command(parent, 'href',
                              args=[latex.Brace(string=link)],
                              string=node.children[0]['content'],
                              info=token.info)
        return h

    @staticmethod
    def _getShortcut(page, key):
        """Helper to find Shortcut tokens added to the page attributes by the Shortcut extension."""
        node = page.get('shortcuts', dict()).get(key)
        if node is not None:
            return node

        raise exceptions.MooseDocsException("Shortcut link key '{}' not found.", key)

class RenderShortcut(components.RenderComponent):
    def createHTML(self, parent, token, page):
        pass

    def createLatex(self, *args):
        pass

class RenderMonospace(components.RenderComponent):
    def createHTML(self, parent, token, page):
        code = html.Tag(parent, 'code')
        html.String(code, content=token['content'], escape=True)
        return code

    def createLatex(self, parent, token, page):
        code = latex.Command(parent, 'texttt', info=token.info)
        latex.String(code, content=token['content'])
        return

class RenderBreak(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.String(parent, content=' ')

    def createLatex(self, parent, token, page):
        return latex.String(parent, content=' ')

class RenderLineBreak(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'br', close=False)
    def createLatex(self, parent, token, page):
        return latex.String(parent, content='\\\\')

class RenderLink(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'a', token, href=token['url'])

    def createLatex(self, parent, token, page):
        url = token['url'].lstrip('#')
        if url.startswith('https://') or url.startswith('http://'):
            cmd = latex.Command(parent, 'href',
                                args=[latex.Brace(string=url)],
                                info=token.info)
        else:
            cmd = latex.Command(parent, 'hyperref',
                                args=[latex.Bracket(string=url, escape=False)],
                                info=token.info)
        return cmd

class RenderParagraph(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'p', token)

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'par', start='\n', end=' ', info=token.info)
        return parent

class RenderOrderedList(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ol', token)

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        if token.get('browser_default', True):
            tag.addClass('browser-default')
        tag['start'] = token['start']
        return tag

    def createLatex(self, parent, token, page):
        return latex.Environment(parent, 'enumerate', after_begin='', info=token.info)

class RenderUnorderedList(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'ul', token)

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        # TODO: accessing token['browser_default'] cause problems, I can't figure out why
        if token.get('browser_default', True):
            tag.addClass('browser-default')
        return tag

    def createLatex(self, parent, token, page):
        return latex.Environment(parent, 'itemize', after_begin='', info=token.info)

class RenderListItem(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'li', token)

    def createLatex(self, parent, token, page):
        latex.Command(parent, 'item', start='\n', end=' ')
        return parent

class RenderString(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.String(parent, content=token['content'], escape=token.get('escape', True))

    def createLatex(self, parent, token, page):
        return latex.String(parent, content=token['content'])

class RenderSpace(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.String(parent, content=' '*token['count'])

    def createLatex(self, parent, token, page):
        return latex.String(parent, content=' '*token['count'])

class RenderQuote(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'blockquote', token)

    def createLatex(self, parent, token, page):
        return latex.Environment(parent, 'quote', after_begin='', info=token.info)

class RenderStrong(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'strong', token)

    def createLatex(self, parent, token, page):
        return latex.Command(parent, 'textbf', info=token.info)

class RenderEmphasis(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'em', token)

    def createLatex(self, parent, token, page):
        return latex.Command(parent, 'emph', info=token.info)

class RenderUnderline(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'u', token)

    def createLatex(self, parent, token, page):
        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'ul', info=token.info)

class RenderStrikethrough(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'strike', token)

    def createLatex(self, parent, token, page):

        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'st', info=token.info)

class RenderSuperscript(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'sup', token)

    def createLatex(self, parent, token, page):
        return latex.Command(parent, 'textsuperscript', info=token.info)

class RenderSubscript(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return html.Tag(parent, 'sub', token)

    def createLatex(self, parent, token, page):
        return latex.Command(parent, 'textsubscript', info=token.info)

class RenderPunctuation(RenderString):
    def createHTML(self, parent, token, page):
        content = token['content']
        if content == '--':
            return html.String(parent, content='&ndash;')
        elif content == '---':
            return html.String(parent, content='&mdash;')

        return RenderString.createHTML(self, parent, token, page)

class RenderError(components.RenderComponent):
    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div', token)
        div.addClass("moose-exception")
        html.String(div, content=token.info[0])
        return div

    def createMaterialize(self, parent, token, page):

        id_ = uuid.uuid4()
        a = html.Tag(parent, 'a', class_="moose-exception modal-trigger", href='#{}'.format(id_))
        if token.info:
            html.String(a, content=token.info[0])

        modal = html.Tag(parent.root, 'div', id_=id_, class_="modal")
        content = html.Tag(modal, 'div', class_="modal-content")
        head = html.Tag(content, 'h2')
        html.String(head, content='Tokenize Error')
        p = html.Tag(content, 'p')

        html.String(p, content=str(token['message']))
        html.Tag(p, 'br', close=False)
        if token.info:
            html.String(p, content='{}:{}'.format(page.local, token.info.line))
        html.Tag(p, 'br', close=False)

        if token.info:
            pre = html.Tag(content, 'pre')
            code = html.Tag(pre, 'code', class_="language-markdown")
            html.String(code, content=token.info[0], escape=True)

        footer = html.Tag(modal, 'div', class_="modal-footer grey lighten-3")
        done = html.Tag(footer, 'a', class_="modal-action modal-close btn-flat")
        html.String(done, content="Done")

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
        return html.Tag(parent, token['tag'], class_='moose-disabled')
