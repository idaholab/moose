#pylint: disable=missing-docstring
import os
import re

import MooseDocs
from MooseDocs import common
from MooseDocs.base import components
from MooseDocs.extensions import core, floats
from MooseDocs.tree import tokens

def make_extension(**kwargs):
    return AutoLinkExtension(**kwargs)

PAGE_LINK_RE = re.compile(r'(?P<filename>.*?\.md)?(?P<bookmark>#.*)?', flags=re.UNICODE)

LocalLink = tokens.newToken('LocalLink', bookmark=u'')
AutoLink = tokens.newToken('AutoLink', page=u'', bookmark=u'')

class AutoLinkExtension(components.Extension):
    """
    Extension that replaces the default Link and LinkShortcut behavior and handles linking to
    other files. This includes the ability to extract the content from the linked page (i.e.,
    headers) for display on the current page.
    """

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        """Replace default core link components on reader and provide auto link rendering."""

        self.requires(core, floats)

        reader.addInline(PageLinkComponent(), location='=LinkInline')
        reader.addInline(PageShortcutLinkComponent(), location='=ShortcutLinkInline')

        renderer.add('LocalLink', RenderLocalLink())
        renderer.add('AutoLink', RenderAutoLink())


def createTokenHelper(key, parent, info, page, use_key_in_modal=False):
    match = PAGE_LINK_RE.search(info[key])
    bookmark = match.group('bookmark')[1:] if match.group('bookmark') else u''
    filename = match.group('filename')

    # The link is local (i.e., [#foo]), the heading will be gathered on render because it
    # could be after the current position.
    if (filename is None) and (bookmark != u''):
        return LocalLink(parent, bookmark=bookmark)

    elif filename is not None:
        return AutoLink(parent, page=filename, bookmark=bookmark)

    else:
        source = common.project_find(info[key])
        if len(source) == 1:
            src = unicode(source[0])
            content = common.fix_moose_header(common.read(os.path.join(MooseDocs.ROOT_DIR, src)))
            code = core.Code(None, language=common.get_language(src), content=content)
            local = src.replace(MooseDocs.ROOT_DIR, '')
            link = floats.create_modal_link(parent, content=code, title=local)
            if use_key_in_modal:
                tokens.String(link, content=os.path.basename(info[key]))
            return link

    return None

class PageShortcutLinkComponent(core.ShortcutLinkInline):
    """
    Creates correct Shortcutlink when *.md files is provide, modal links when a source files is
    given, otherwise a core.ShortcutLink token.
    """

    def createToken(self, parent, info, page):
        token = createTokenHelper('key', parent, info, page, use_key_in_modal=True)
        if token is None:
            return core.ShortcutLinkInline.createToken(self, parent, info, page)
        return token

class PageLinkComponent(core.LinkInline):
    """
    Creates correct link when *.md files is provide, modal links when a source files is given,
    otherwise a core.Link token.
    """

    def createToken(self, parent, info, page):
        token = createTokenHelper('url', parent, info, page)
        if token is None:
            return core.LinkInline.createToken(self, parent, info, page)
        return token

class RenderLinkBase(components.RenderComponent):

    def createHTMLHelper(self, parent, token, page, desired):
        bookmark = token['bookmark']

        url = unicode(desired.relativeDestination(page))
        if bookmark:
            url += '#{}'.format(bookmark)

        link = core.Link(None, url=url)
        if not token.children:
            ast = self.getSyntaxTree(desired)
            heading = common.find_heading(desired, ast, bookmark)
            if heading is not None:
                for child in heading:
                    child.parent = link
            else:
                tokens.String(link, content=url)
        else:
            for child in token:
                child.parent = link

        self.renderer.render(parent, link, page)
        return parent

class RenderLocalLink(RenderLinkBase):
    """
    Creates a link to a local item.
    """
    def createHTML(self, parent, token, page):
        return self.createHTMLHelper(parent, token, page, page)

    def createLatex(self, parent, token, page):
        pass

class RenderAutoLink(RenderLinkBase):
    """
    Create link to another page and extract the heading for the text, if no children provided.
    """
    def createHTML(self, parent, token, page):
        desired = self.findPage(token['page'])
        return self.createHTMLHelper(parent, token, page, desired)

    def createLatex(self, parent, token, page):
        pass
