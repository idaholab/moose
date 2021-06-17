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
import logging
import moosetree
import mooseutils

import MooseDocs
from .. import common
from ..common import exceptions
from ..base import components, Extension
from ..tree import tokens, latex
from . import core, floats, heading, modal

def make_extension(**kwargs):
    return AutoLinkExtension(**kwargs)

PAGE_LINK_RE = re.compile(r'(?P<filename>.*?\.md)?(?P<bookmark>#.*)?', flags=re.UNICODE)
LOG = logging.getLogger(__name__)

LocalLink = tokens.newToken('LocalLink', bookmark=None)
AutoLink = tokens.newToken('AutoLink', page='', bookmark=None, optional=False, warning=False,
                           exact=False)

class AutoLinkExtension(Extension):
    """
    Extension that replaces the default Link and LinkShortcut behavior and handles linking to
    other files. This includes the ability to extract the content from the linked page (i.e.,
    headers) for display on the current page.
    """

    @staticmethod
    def defaultConfig():
        config = Extension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        """Replace default core link components on reader and provide auto link rendering."""

        self.requires(core, floats, heading)

        reader.addInline(PageLinkComponent(), location='=LinkInline')
        reader.addInline(PageShortcutLinkComponent(), location='=ShortcutLinkInline')

        renderer.add('LocalLink', RenderLocalLink())
        renderer.add('AutoLink', RenderAutoLink())

def createTokenHelper(key, parent, info, page, optional, exact, language):
    match = PAGE_LINK_RE.search(info[key])
    bookmark = match.group('bookmark')[1:] if match.group('bookmark') else None
    filename = match.group('filename') or None

    # The link is local (i.e., [#foo]), the heading will be gathered on render because it
    # could be after the current position.
    if (filename is None) and (bookmark is not None):
        return LocalLink(parent, bookmark=bookmark)
    elif (filename is not None):
        return AutoLink(parent, page=filename, bookmark=bookmark, optional=optional, exact=exact)
    elif common.project_find(info[key]):
        return modal.ModalSourceLink(parent, src=info[key], language=language)
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
        settings['exact'] = (False, "Enable/disable exact match for markdown file.")
        settings['language'] = (None, "The language used for source file syntax highlighting.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('key', parent, info, page, self.settings['optional'],
                                  self.settings['exact'], self.settings['language'])
        return token or core.ShortcutLinkInline.createToken(self, parent, info, page)

class PageLinkComponent(core.LinkInline):
    """
    Creates correct link when *.md files is provide, modal links when a source files is given,
    otherwise a core.Link token.
    """

    @staticmethod
    def defaultSettings():
        settings = core.LinkInline.defaultSettings()
        settings['optional'] = (False, "Toggle the link as optional when file doesn't exist.")
        settings['exact'] = (False, "Enable/disable exact match for markdown file.")
        settings['language'] = (None, "The language used for source file syntax highlighting.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('url', parent, info, page, self.settings['optional'],
                                  self.settings['exact'], self.settings['language'])
        return token or core.LinkInline.createToken(self, parent, info, page)

class RenderLinkBase(components.RenderComponent):

    def createHTMLHelper(self, parent, token, page, desired):
        bookmark = token['bookmark']

        # Handle 'optional' linking
        if desired is None:
            self._createOptionalContent(parent, token, page)
            return None

        url = str(desired.relativeDestination(page))
        if bookmark:
            url += '#{}'.format(bookmark)

        link = core.Link(None, url=url, info=token.info)
        if len(token.children) == 0:
            head = heading.find_heading(desired, bookmark)

            if head is not None:
                head.copyToToken(link)
            else:
                link['class'] = 'moose-error'
                tokens.String(link, content=url)
                msg = "Unable to locate local heading with URL '{}'".format(url)
                LOG.error(common.report_error(msg, page.source,
                                              token.info.line if token.info else None,
                                              token.info[0] if token.info else token.text()))
        else:
            token.copyToToken(link)

        self.renderer.render(parent, link, page)
        return None

    def createLatexHelper(self, parent, token, page, desired):
        func = lambda p, t, u, l: latex.Command(p, 'hyperref', token=t,
                                                args=[latex.Bracket(string=l)])
        # Create optional content
        bookmark = token['bookmark']

        if desired is None:
            self._createOptionalContent(parent, token, page)
            return None

        url = str(desired.relativeDestination(page))
        head = heading.find_heading(desired, bookmark)

        tok = tokens.Token(None)
        if head is None:
            msg = "The linked page ({}) does not contain a heading, so the filename " \
                  "is being utilized.".format(desired.local)
            LOG.warning(common.report_error(msg, page.source,
                                            token.info.line if token.info else None,
                                            token.info[0] if token.info else token.text(),
                                            prefix='WARNING'))
            latex.String(parent, content=page.local)

        else:
            label = head.get('id') or re.sub(r' +', r'-', head.text().lower())
            href = func(parent, token, url, label)

            if len(token) == 0:
                head.copyToToken(tok)
            else:
                token.copyToToken(tok)

            self.renderer.render(href, tok, page)
        return None

    def _createOptionalContent(self, parent, token, page):
        """Renders text without link for optional link."""
        tok = tokens.Token(None)
        token.copyToToken(tok)
        if len(tok) == 0: # Use filename if no children exist
            tokens.String(tok, content=token['page'])
        self.renderer.render(parent, tok, page)

class RenderLocalLink(RenderLinkBase):
    """
    Creates a link to a local item.
    """
    def createHTML(self, parent, token, page):
        return self.createHTMLHelper(parent, token, page, page)

    def createLatex(self, parent, token, page):
        return self.createLatexHelper(parent, token, page, page)

class RenderAutoLink(RenderLinkBase):
    """
    Create link to another page and extract the heading for the text, if no children provided.
    """
    def createHTML(self, parent, token, page):
        desired = self.translator.findPage(token['page'],
                                           throw_on_zero=not token['optional'],
                                           exact=token['exact'],
                                           warn_on_zero=token['warning'])
        return self.createHTMLHelper(parent, token, page, desired)

    def createLatex(self, parent, token, page):
        desired = self.translator.findPage(token['page'],
                                           throw_on_zero=not token['optional'],
                                           warn_on_zero=token['warning'],
                                           exact=token['exact'])
        return self.createLatexHelper(parent, token, page, desired)
