"""
Defines the "core" extension for translating MooseDocs markdown to HTML and LaTeX.
"""
import re
import uuid
import logging

import anytree

from MooseDocs.base import components, renderers
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    """
    Create and return the CoreExtension object for converting from markdown to html/latex.
    """
    return CoreExtension(**kwargs)

class CoreExtension(components.Extension):
    """
    The core markdown extension object. Extension objects add the tokenize and rendering components.
    """

    def reinit(self):
        tokens.CountToken.COUNTS.clear()


    def extend(self, reader, renderer):
        """
        Add the extension components.
        """

        # Block tokenize components
        reader.addBlock(Code())
        reader.addBlock(Quote())
        reader.addBlock(HeadingHash())
        reader.addBlock(OrderedList())
        reader.addBlock(UnorderedList())
        reader.addBlock(Shortcut())
        reader.addBlock(Paragraph())
        reader.addBlock(EndOfFile())

        # Inline tokenize components
        reader.addInline(Format())
        reader.addInline(Link())
        reader.addInline(ShortcutLink())
        reader.addInline(Punctuation())
        reader.addInline(Number())
        reader.addInline(Word())
        reader.addInline(Break())
        reader.addInline(Space())

        # Render components
        renderer.add(tokens.Heading, RenderHeading())
        renderer.add(tokens.Code, RenderCode())
        renderer.add(tokens.ShortcutLink, RenderShortcutLink())
        renderer.add(tokens.Shortcut, RenderShortcut())
        renderer.add(tokens.Monospace, RenderMonospace())
        renderer.add(tokens.Break, RenderBreak())
        renderer.add(tokens.ErrorToken, RenderError())
        renderer.add(tokens.ExceptionToken, RenderException())

        renderer.add(tokens.Link, RenderLink())
        renderer.add(tokens.Paragraph, RenderParagraph())
        renderer.add(tokens.UnorderedList, RenderUnorderedList())
        renderer.add(tokens.OrderedList, RenderOrderedList())
        renderer.add(tokens.ListItem, RenderListItem())
        renderer.add(tokens.Quote, RenderQuote())
        renderer.add(tokens.Strong, RenderStrong())
        renderer.add(tokens.Underline, RenderUnderline())
        renderer.add(tokens.Emphasis, RenderEmphasis())
        renderer.add(tokens.Strikethrough, RenderStrikethrough())
        renderer.add(tokens.Superscript, RenderSuperscript())
        renderer.add(tokens.Subscript, RenderSubscript())
        renderer.add(tokens.Label, RenderLabel())
        renderer.add(tokens.Punctuation, RenderPunctuation())

        for t in [tokens.Word, tokens.Space, tokens.Number, tokens.String]:
            renderer.add(t, RenderString())

        #TODO: Make a generic preamble method?
        if isinstance(renderer, renderers.LatexRenderer):
            renderer.addPackage(u'amsmath')
            renderer.addPackage(u'soul')
            renderer.addPackage(u'hyperref')

# Documenting all these classes is far to repetitive and useless.
#pylint: disable=missing-docstring

#TODO: Rename these classes to use the word compoment so they don't get mixed with tokens names
class Code(components.TokenComponent):
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

    def createToken(self, info, parent):
        args = info['settings'].split()
        if args and ('=' not in args[0]):
            self.settings['language'] = args[0]

        return tokens.Code(parent, code=info['code'], language=self.settings['language'],
                           **self.attributes)

class Quote(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})'         # start of string or empty line
                    r'(?P<quote>^>[ $].*?)'  # quote content
                    r'(?=\n*\Z|\n{2,})',        # end of string or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, info, parent):
        content = []
        for line in info['quote'].rstrip('\n').split('\n'):
            if line == u'>':
                content.append('')
            elif line.startswith(u'> '):
                content.append(line[2:])
            else:
                raise Exception(repr(line))

        #TODO: error check that all lines begin with '> '
        quote = tokens.Quote(parent)
        self.reader.parse(quote, '\n'.join(content))
        return quote

class HeadingHash(components.TokenComponent):
    """
    Hash style markdown headings with settings.

    # Heading Level One with=settings
    """
    TOKEN = tokens.Heading
    RE = re.compile(r'(?:\A|\n{2,})'             # start of string or empty line
                    r'^(?P<level>#{1,6}) '       # match 1 to 6 #'s at the beginning of line
                    r'(?P<inline>.*?) *'         # heading text that will be inline parsed
                    r'(?P<settings>\s+\w+=.*?)?' # optional match key, value settings
                    r'(?=\n*\Z|\n{2,})',         # match up to end of string or newline(s)
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, info, parent):
        content = info['inline']
        heading = tokens.Heading(parent, level=info['level'].count('#'), **self.attributes)
        tokens.Label(heading, text=content)
        return heading

class List(components.TokenComponent):
    RE = None
    ITEM_RE = None
    TOKEN = None

    def createToken(self, info, parent):
        marker = info['marker']
        n = len(marker)
        token = self.TOKEN(parent) #pylint: disable=not-callable
        strip_regex = re.compile(r'^ {%s}(.*?)$' % n, flags=re.MULTILINE)

        for item in self.ITEM_RE.finditer(info['items']):
            content = ' '*n + item.group('item')
            indent = re.search(r'^\S', content, flags=re.MULTILINE|re.UNICODE)
            if indent:
                msg = "List item content must be indented by {} to match the list item " \
                       "characters of '{}', to end a list item you must use two empty lines."
                raise exceptions.TokenizeException(msg.format(n, marker))

            content = strip_regex.sub(r'\1', content)
            self.reader.parse(tokens.ListItem(token), content)

        return token

class UnorderedList(List):
    RE = re.compile(r'(?:\A|\n{2,})'                   # start of string or empty line
                    r'(?P<items>(?P<marker>^- ).*?)'   # all items
                    r'(?=\n{3,}|\n*\Z|\n{2}^[^-\s])',  # stop with 2 empty or 1 not with marker
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    ITEM_RE = re.compile(r'^- (?P<item>.*?)(?=\Z|^- )', flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    TOKEN = tokens.UnorderedList

class OrderedList(List):
    """Ordered lists."""
    RE = re.compile(r'(?:\A|\n{2,})'                        # start of string or empty line
                    r'(?P<items>(?P<marker>^[0-9]+\. ).*?)' # all items
                    r'(?=\n{3,}|\n*\Z|\n{2}^[^[0-9\s])',    # stop with 2 empty or 1 not with marker
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    ITEM_RE = re.compile(r'^[0-9]+\. (?P<item>.*?)(?=\Z|^[0-9]+\. )',
                         flags=re.MULTILINE|re.DOTALL|re.UNICODE)
    TOKEN = tokens.OrderedList

    #TODO: figure out how to handle settings???
    #TODO: combine regex for ordered/unordered
    # 1. ::start=42 type=a:: This is the actual content.???

    @staticmethod
    def defaultSettings():
        settings = List.defaultSettings()
        settings['type'] = ('1', "The list type (1, A, a, i, or I).")
        return settings

    def createToken(self, info, parent):
        token = List.createToken(self, info, parent)
        token.start = int(info['marker'].strip('. '))
        return token

class Shortcut(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})^\[(?P<key>\w+)\]: ' # shortcut key
                    r'(?P<link>.*?)'                    # shortcut link
                    r'(?=\Z|\n{2,})',                   # stop new line or end of file
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, info, parent):
        return tokens.Shortcut(parent, key=info['key'], link=info['link'], string=info['key'])

class Paragraph(components.TokenComponent):
    RE = re.compile(r'(?:\A|\n{2,})'   # start of string of empty line
                    r'(?P<inline>.*?)' # content
                    r'(?=\Z|\n{2,})',  # stop with end or empty line
                    flags=re.MULTILINE|re.DOTALL|re.UNICODE)

    def createToken(self, info, parent): #pylint: disable=unused-argument
        return tokens.Paragraph(parent)

class EndOfFile(components.TokenComponent):
    RE = re.compile(r'.*', flags=re.UNICODE|re.MULTILINE|re.DOTALL)
    def createToken(self, info, parent):
        return parent

class Link(components.TokenComponent):
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

    def createToken(self, info, parent):
        return tokens.Link(parent, url=info['url'], **self.attributes)

class ShortcutLink(components.TokenComponent):
    RE = re.compile(r'\['                        # opening [
                    r'(?P<key>.+?)'              # key
                    r'(?:\s+(?P<settings>.*?))?' # settings
                    r'\]',                       # closing ]
                    flags=re.UNICODE)
    def createToken(self, info, parent):
        tokens.ShortcutLink(parent, key=info['key'], **self.attributes)
        return parent

class Break(components.TokenComponent):
    RE = re.compile(r'(?P<break>\n+)')
    def createToken(self, info, parent):
        tokens.Break(parent, count=len(info['break']))
        return parent

class Space(components.TokenComponent):
    RE = re.compile(r'(?P<space> +)')
    def createToken(self, info, parent):
        tokens.Space(parent, count=len(info['space']))
        return parent

class Punctuation(components.TokenComponent):
    RE = re.compile(r'(([^A-Za-z0-9\s])\2*)')
    def createToken(self, info, parent):
        tokens.Punctuation(parent, content=info[0])
        return parent

class Number(components.TokenComponent):
    RE = re.compile(r'([0-9]+)')
    def createToken(self, info, parent):
        tokens.Number(parent, content=info[0])
        return parent

class Word(components.TokenComponent):
    RE = re.compile(r'([A-Za-z]+)')
    def createToken(self, info, parent):
        tokens.Word(parent, content=info[0])
        return parent

class Format(components.TokenComponent):
    RE = re.compile(r'(?P<token>[\@|\^|\=|\*|\+|~`])(?=\S)(?P<inline>.*?)(?<=\S)(?:\1)',
                    flags=re.MULTILINE|re.DOTALL|re.DOTALL)

    def createToken(self, info, parent):
        tok = info['token']

        # Sub/super script must have word before the rest cannot
        if (tok == '^') or (tok == '@'):
            if not parent.children or not isinstance(parent.children[-1], tokens.Word):
                return None
        elif parent.children and isinstance(parent.children[-1], tokens.Word):
            return None

        if tok == u'@':
            return tokens.Subscript(parent)
        elif tok == u'^':
            return tokens.Superscript(parent)
        elif tok == u'=':
            return tokens.Underline(parent)
        elif tok == u'*':
            return tokens.Emphasis(parent)
        elif tok == u'+':
            return tokens.Strong(parent)
        elif tok == u'~':
            return tokens.Strikethrough(parent)
        elif tok == u'`':
            return tokens.Monospace(parent, code=info['inline'], recursive=False)

####################################################################################################
# Rendering.
####################################################################################################

class RenderHeading(components.RenderComponent):
    LATEX_SECTIONS = ['part', 'chapter', 'section', 'subsection', 'subsubsection', 'paragraph',
                      'subparagraph']

    def createHTML(self, token, parent): #pylint: disable=no-self-use
        attr = token.attributes
        id_ = attr.get('id')
        if not id_:
            func = lambda n: isinstance(n, tokens.Word)
            words = [node.content.lower() for node in anytree.search.findall(token, filter_=func)]
            attr['id'] = '-'.join(words)

        return html.Tag(parent, 'h{}'.format(token.level), **attr)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, self.LATEX_SECTIONS[token.level], start='\n')

class RenderLabel(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use,unused-argument
        pass

    def createLatex(self, token, parent): #pylint: disable=no-self-use
        label = token.get('id', re.sub(r' +', r'-', token.text.lower()))
        return latex.Command(parent, 'label', string=label)

class RenderCode(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        language = 'language-{}'.format(token.language)
        pre = html.Tag(parent, 'pre', **token.attributes)
        code = html.Tag(pre, 'code', class_=language)
        html.String(code, content=token.code, escape=token.escape)
        return pre

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'verbatim', string=token.code, after_begin='',
                                 before_end='')

class RenderShortcutLink(components.RenderComponent):
    def __init__(self, *args, **kwargs):
        components.RenderComponent.__init__(self, *args, **kwargs)
        self.__cache = dict()

    def createHTML(self, token, parent):
        a = html.Tag(parent, 'a', **token.attributes)
        node = self.getShortcut(token)
        a['href'] = node.link
        for child in node.children:
            self.translator.renderer.process(a, child)
        return a

    def createMaterialize(self, token, parent):
        tag = self.createHTML(token, parent)
        tag.addClass('tooltipped')
        tag['data-tooltip'] = tag['href']
        tag['data-position'] = 'top'
        return tag

    def createLatex(self, token, parent):
        cmd = latex.CustomCommand(parent, 'href')
        node = self.getShortcut(token)

        latex.Brace(cmd, string=node.link)
        latex.Brace(cmd, string=node.string)
        return cmd

    def getShortcut(self, token):
        if token.key in self.__cache:
            return self.__cache[token.key]

        #TODO: error if more than one found
        for node in anytree.PreOrderIter(token.root, maxlevel=None):
            if isinstance(node, tokens.Shortcut) and node.key == token.key:
                self.__cache[token.key] = node
                return node

        msg = "The shortcut link key '{}' was not located in the list of shortcuts."
        raise exceptions.RenderException(token.info, msg, token.key)

class RenderShortcut(components.RenderComponent):
    def createHTML(self, token, parent):
        pass

    def createLatex(self, *args):
        pass

class RenderMonospace(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        code = html.Tag(parent, 'code')
        html.String(code, content=token.code, escape=True)
        return code

    def createLatex(self, token, parent): #pylint: disable=no-self-use
        code = latex.Command(parent, 'texttt')
        latex.String(code, content=token.code)
        return

class RenderBreak(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return html.String(parent, content=u' ')

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.String(parent, content=u' ')

class RenderLink(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'a', href=token.url, **token.attributes)

    def createMaterialize(self, token, parent):
        tag = self.createHTML(token, parent)
        if token.tooltip:
            tag.addClass('tooltipped')
            tag['data-tooltip'] = tag['href']
            tag['data-position'] = 'top'
        return tag

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        url = token.url.lstrip('#')
        cmd = latex.CustomCommand(parent, 'href')
        latex.Brace(cmd, string=url)
        arg1 = latex.Brace(cmd)
        return arg1

class RenderParagraph(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return html.Tag(parent, 'p', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        latex.CustomCommand(parent, 'par', start='\n', end=' ')
        return parent

class RenderOrderedList(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return html.Tag(parent, 'ol', **token.attributes)

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use
        tag = self.createHTML(token, parent)
        tag.addClass('browser-default')
        tag['start'] = token.start
        return tag

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'enumerate')

class RenderUnorderedList(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'ul', **token.attributes)

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use
        tag = self.createHTML(token, parent)
        tag.addClass('browser-default')
        return tag

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'itemize')

class RenderListItem(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'li', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        latex.Command(parent, 'item')
        return parent

class RenderString(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.String(parent, content=token.content)

    def createLatex(self, token, parent): #pylint: disable=no-self-use
        return latex.String(parent, content=token.content)

class RenderQuote(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'blockquote', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Environment(parent, 'quote', after_begin='')

class RenderStrong(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'strong', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textbf')

class RenderEmphasis(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'em', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'emph')

class RenderUnderline(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'u', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'ul')

class RenderStrikethrough(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'strike', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument

        for n in parent.path:
            if n.name in ['so', 'ul']:
                msg = "Nested strikethrough and underline commands are not supported in LaTeX, " \
                      "see the Soul package for details."
                LOG.warning(msg)
                return parent

        return latex.Command(parent, 'so')

class RenderSuperscript(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'sup', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textsuperscript')

class RenderSubscript(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'sub', **token.attributes)

    def createLatex(self, token, parent): #pylint: disable=no-self-use,unused-argument
        return latex.Command(parent, 'textsubscript')

class RenderPunctuation(RenderString):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        if token.content == u'--':
            return html.String(parent, content=u'&ndash;')
        elif token.content == u'---':
            return html.String(parent, content=u'&mdash;')

        return RenderString.createHTML(self, token, parent)

class RenderError(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', class_="moose-exception", **token.attributes)
        html.String(div, content=token.info[0])
        return div

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use

        id_ = uuid.uuid4()
        a = html.Tag(parent, 'a', class_="moose-exception modal-trigger", href='#{}'.format(id_))
        html.String(a, content=token.info[0])

        modal = html.Tag(parent.root, 'div', id_=id_, class_="modal")
        content = html.Tag(modal, 'div', class_="modal-content")
        head = html.Tag(content, 'h2')
        html.String(head, content=u'Tokenize Error')
        p = html.Tag(content, 'p')

        html.String(p, content=unicode(token.message))
        if self.translator.current:
            html.Tag(p, 'br', close=False)
            html.String(p, content=u'{}:{}'.format(self.translator.current.local, token.info.line))
        html.Tag(p, 'br', close=False)

        pre = html.Tag(content, 'pre')
        code = html.Tag(pre, 'code', class_="language-markdown")
        html.String(code, content=token.info[0], escape=True)

        footer = html.Tag(modal, 'div', class_="modal-footer grey lighten-3")
        done = html.Tag(footer, 'a', class_="modal-action modal-close btn-flat")
        html.String(done, content=u"Done")

        return content

    def createLatex(self, token, parent):
        pass

class RenderException(RenderError):

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use
        content = RenderError.createMaterialize(self, token, parent)

        pre = html.Tag(content, 'pre', style="font-size:80%;")
        html.String(pre, content=unicode(token.traceback), escape=True)

        return content
