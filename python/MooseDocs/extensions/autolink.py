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
import logging
import anytree
import MooseDocs
from MooseDocs import common
from MooseDocs.base import components
from MooseDocs.extensions import core, floats, heading
from MooseDocs.tree import tokens, latex

def make_extension(**kwargs):
    return AutoLinkExtension(**kwargs)

PAGE_LINK_RE = re.compile(r'(?P<filename>.*?\.md)?(?P<bookmark>#.*)?', flags=re.UNICODE)
LOG = logging.getLogger(__name__)

SourceLink = tokens.newToken('SourceLink')
LocalLink = tokens.newToken('LocalLink', bookmark=u'')
AutoLink = tokens.newToken('AutoLink', page=u'', bookmark=u'', optional=False, warning=False,
                           exact=False)

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
        renderer.add('SourceLink', RenderSourceLink())

def createTokenHelper(key, parent, info, page, use_key_in_modal=False, optional=False, exact=False):
    match = PAGE_LINK_RE.search(info[key])
    bookmark = match.group('bookmark')[1:] if match.group('bookmark') else u''
    filename = match.group('filename')

    # The link is local (i.e., [#foo]), the heading will be gathered on render because it
    # could be after the current position.
    if (filename is None) and (bookmark != u'' or (match.group('bookmark') == u'#')):
        return LocalLink(parent, bookmark=bookmark)

    elif filename is not None:
        return AutoLink(parent, page=filename, bookmark=bookmark, optional=optional, exact=exact)

    else:
        source = common.project_find(info[key])
        if len(source) == 1:
            src_link = SourceLink(parent)
            src = str(source[0])
            content = common.fix_moose_header(common.read(os.path.join(MooseDocs.ROOT_DIR, src)))
            code = core.Code(None, language=common.get_language(src), content=content)
            local = src.replace(MooseDocs.ROOT_DIR, '')
            link = floats.create_modal_link(src_link, content=code, title=local)
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
        settings['exact'] = (False, "Enable/disable exact match for markdown file.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('key', parent, info, page,
                                  use_key_in_modal=True,
                                  optional=self.settings['optional'],
                                  exact=self.settings['exact'])
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
        settings['exact'] = (False, "Enable/disable exact match for markdown file.")
        return settings

    def createToken(self, parent, info, page):
        token = createTokenHelper('url', parent, info, page, optional=self.settings['optional'],
                                  exact=self.settings['exact'])
        if token is None:
            return core.LinkInline.createToken(self, parent, info, page)
        return token

class RenderLinkBase(components.RenderComponent):

    def createHTMLHelper(self, parent, token, page, desired):
        bookmark = token['bookmark']

        # Handle 'optional' linking
        if desired is None:
            self._createOptionalContent(parent, token, page)
            return None

        if desired is page:
            url = '#{}'.format(bookmark) if bookmark else '#'
        else:
            url = str(desired.relativeDestination(page))
            if bookmark:
                url += '#{}'.format(bookmark)

        link = core.Link(None, url=url, info=token.info)
        if len(token.children) == 0:
            head = None
            if desired is page:
                for n in anytree.PreOrderIter(token.root,
                                              filter_=lambda n: bookmark == n.get('id', None)):
                    head = n
                    break
            else:
                head = heading.find_heading(self.translator, desired, bookmark)

            if head is not None:
                head.copyToToken(link)
            else:
                link['class'] = 'moose-error'
                tokens.String(link, content=url)
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
        head = heading.find_heading(self.translator, desired, bookmark)

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
            tokens.String(tok, content=page.local)
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
                                           warn_on_zero=token['warning'],
                                           exact=token['exact'])
        return self.createHTMLHelper(parent, token, page, desired)

    def createLatex(self, parent, token, page):
        desired = self.translator.findPage(token['page'],
                                           throw_on_zero=not token['optional'],
                                           warn_on_zero=token['warning'],
                                           exact=token['exact'])
        return self.createLatexHelper(parent, token, page, desired)

class RenderSourceLink(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return parent

    def createLatex(self, parent, token, page):
        root = tokens.Token(None)
        token(0).copyToToken(root)
        self.renderer.render(parent, root, page)
        return None
