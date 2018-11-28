#pylint: disable=missing-docstring
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

import MooseDocs
from MooseDocs import common
from MooseDocs.base import components
from MooseDocs.extensions import core, floats, heading
from MooseDocs.tree import tokens

def make_extension(**kwargs):
    return AutoLinkExtension(**kwargs)

PAGE_LINK_RE = re.compile(r'(?P<filename>.*?\.md)?(?P<bookmark>#.*)?', flags=re.UNICODE)

LocalLink = tokens.newToken('LocalLink', bookmark=u'')
AutoLink = tokens.newToken('AutoLink', page=u'', bookmark=u'', optional=False)

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

        self.requires(core, floats, heading)

        reader.addInline(PageLinkComponent(), location='=LinkInline')
        reader.addInline(PageShortcutLinkComponent(), location='=ShortcutLinkInline')

        renderer.add('LocalLink', RenderLocalLink())
        renderer.add('AutoLink', RenderAutoLink())

def createTokenHelper(key, parent, info, page, use_key_in_modal=False, optional=False):
    match = PAGE_LINK_RE.search(info[key])
    bookmark = match.group('bookmark')[1:] if match.group('bookmark') else u''
    filename = match.group('filename')

    # The link is local (i.e., [#foo]), the heading will be gathered on render because it
    # could be after the current position.
    if (filename is None) and (bookmark != u''):
        return LocalLink(parent, bookmark=bookmark)

    elif filename is not None:
        return AutoLink(parent, page=filename, bookmark=bookmark, optional=optional)

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

    @staticmethod
    def defaultSettings():
        settings = core.ShortcutLinkInline.defaultSettings()
        settings['optional'] = (False, "Toggle the link as optional when file doesn't exist.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('key', parent, info, page,
                                  use_key_in_modal=True,
                                  optional=self.settings['optional'])
        if token is None:
            return core.ShortcutLinkInline.createToken(self, parent, info, page)
        return token

class PageLinkComponent(core.LinkInline):
    """
    Creates correct link when *.md files is provide, modal links when a source files is given,
    otherwise a core.Link token.
    """

    @staticmethod
    def defaultSettings():
        settings = core.LinkInline.defaultSettings()
        settings['optional'] = (False, "Toggle the link as optional when file doesn't exist.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('url', parent, info, page, optional=self.settings['optional'])
        if token is None:
            return core.LinkInline.createToken(self, parent, info, page)
        return token

class RenderLinkBase(components.RenderComponent):

    def createHTMLHelper(self, parent, token, page, desired):
        bookmark = token['bookmark']

        # Handle 'optional' linking
        if desired is None:
            tok = tokens.Token(None)
            for child in token.copy():
                child.parent = tok
            self.renderer.render(parent, tok, page)
            return None

        url = unicode(desired.relativeDestination(page))
        if bookmark:
            url += '#{}'.format(bookmark)

        link = core.Link(None, url=url, info=token.info)
        if len(token.children) == 0:
            head = heading.find_heading(self.translator, desired, bookmark)

            if head is not None:
                for child in head:
                    child.parent = link
            else:
                tokens.String(link, content=url)

        else:
            for child in token.copy():
                child.parent = link

        self.renderer.render(parent, link, page)
        return None

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
        desired = self.translator.findPage(token['page'], throw_on_zero=not token['optional'])
        return self.createHTMLHelper(parent, token, page, desired)

    def createLatex(self, parent, token, page):
        pass
