#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re
import logging

import MooseDocs
from .. import common
from ..base import components, Extension
from ..tree import tokens, latex, html
from . import core, floats, heading, modal

def make_extension(**kwargs):
    return AutoLinkExtension(**kwargs)

PAGE_LINK_RE = re.compile(r'(?P<filename>^(?!http).*?\.md)?(?P<bookmark>#.*)?', flags=re.UNICODE)
LOG = logging.getLogger(__name__)

LocalLink = tokens.newToken('LocalLink', bookmark=None)
AutoLink = tokens.newToken('AutoLink', page='', bookmark=None, alternative=None, optional=False,
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

        self.requires(core, heading, modal)

        reader.addInline(PageLinkComponent(), location='=LinkInline')
        reader.addInline(PageShortcutLinkComponent(), location='=ShortcutLinkInline')

        renderer.add('LocalLink', RenderLocalLink())
        renderer.add('AutoLink', RenderAutoLink())

def createTokenHelper(key, parent, info, page, settings):
    match = PAGE_LINK_RE.search(info[key])
    bookmark = match.group('bookmark')[1:] if match.group('bookmark') else None
    filename = match.group('filename')

    # The link is local (i.e., [#foo]), the heading will be gathered on render because it
    # could be after the current position.
    if (filename is None) and (bookmark is not None):
        return LocalLink(parent, bookmark=bookmark)
    elif (filename is not None):
        return AutoLink(parent, page=filename, bookmark=bookmark, optional=settings['optional'],
                        exact=settings['exact'], alternative=settings['alternative'])
    elif common.project_find(info[key]):
        return modal.ModalSourceLink(parent, src=common.check_filenames(info[key]),
                                     language=settings['language'])
    return None

class PageShortcutLinkComponent(core.ShortcutLinkInline):
    """
    Creates correct Shortcutlink when *.md files is provide, modal links when a source files is
    given, otherwise a core.ShortcutLink token.
    """

    @staticmethod
    def defaultSettings():
        settings = core.ShortcutLinkInline.defaultSettings()
        settings['alternative'] = (None, "An alternative link to use when the file doesn't exist.")
        settings['optional'] = (False, "Toggle the link as optional when the file doesn't exist.")
        settings['exact'] = (False, "Enable/disable exact match for the markdown file.")
        settings['language'] = (None, "The language used for source file syntax highlighting.")
        return settings

    def createToken(self, parent, info, page, settings):
        token = createTokenHelper('key', parent, info, page, settings)
        return token or core.ShortcutLinkInline.createToken(self, parent, info, page, settings)

class PageLinkComponent(core.LinkInline):
    """
    Creates correct link when *.md files is provide, modal links when a source files is given,
    otherwise a core.Link token.
    """

    @staticmethod
    def defaultSettings():
        settings = core.LinkInline.defaultSettings()
        settings['alternative'] = (None, "An alternative link to use when the file doesn't exist.")
        settings['optional'] = (False, "Toggle the link as optional when the file doesn't exist.")
        settings['exact'] = (False, "Enable/disable exact match for the markdown file.")
        settings['language'] = (None, "The language used for source file syntax highlighting.")
        return settings

    def createToken(self, parent, info, page, settings):
        token = createTokenHelper('url', parent, info, page, settings)
        return token or core.LinkInline.createToken(self, parent, info, page, settings)

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
                                                args=[latex.Bracket(string=l, escape=False)])
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
        alternative = token['alternative']
        optional = token['optional']
        exact = token['exact']
        try:
            desired = self.translator.findPage(token['page'], exact=exact,
                                               throw_on_zero=not optional and alternative is None)
        except MooseDocs.common.exceptions.MooseDocsException:
            html.String(parent, content=token['page'], class_='moose_error')
            raise

        # If no page was found, create a new copy of the token and render the alernative hyperlink
        if (desired is None) and (alternative is not None):
            token = token.copy(info=True)
            match = PAGE_LINK_RE.search(alternative)
            token['bookmark'] = match.group('bookmark')[1:] if match.group('bookmark') else None
            token['page'] = match.group('filename')

            # Determine what to do with the alternative token. If no filename was provided, it could
            # be a local link or a URL, otherwise, we'll search for the 'filename#bookmark' href.
            if token['page'] is None:
                if token['bookmark'] is not None:
                    return RenderLocalLink.createHTML(self, parent, token, page)
                elif len(token):
                    token['url'] = alternative
                    return core.RenderLink.createHTML(self, parent, token, page)
                else:
                    msg = "URLs cannot be used as an alternative for automatic shortcut links. " \
                          "Please use the '[text](link alternative=foo)' syntax instead."
                    LOG.error(common.report_error(msg, page.source,
                                                  token.info.line if token.info else None,
                                                  token.info[0] if token.info else token.text()))
                    return None
            desired = self.translator.findPage(token['page'], exact=exact, throw_on_zero=not optional)

        return self.createHTMLHelper(parent, token, page, desired)

    def createLatex(self, parent, token, page):
        throw = not token['optional'] and token['alternative'] is None
        desired = self.translator.findPage(token['page'], exact=token['exact'], throw_on_zero=throw)

        # The 'alternative' token is not supported here as it doesn't seem appropriate [crswong888]
        if desired is None and token['alternative'] is not None:
            msg = "Warning: The 'alternative' setting for automatic links has no effect on LaTeX " \
                  "renderers." + ("\n" + token.info[0] if token.info else "")
            latex.String(parent, content=msg)
            return None

        return self.createLatexHelper(parent, token, page, desired)
