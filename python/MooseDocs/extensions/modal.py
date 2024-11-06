#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import uuid
import logging
import fnmatch

import mooseutils
import moosetree
import collections
import MooseDocs
from .. import common
from ..common import exceptions, report_error
from ..base import components
from ..tree import tokens, html, latex
from . import core, command

LOG = logging.getLogger('MooseDocs.extensions.modal')

def make_extension(**kwargs):
    return ModalExtension(**kwargs)

ModalLink = tokens.newToken('ModalLink', content=None, title=None)
ModalSourceLink = tokens.newToken('ModalSourceLink', src=None, title=None, language=None, link_prefix=None)

class ModalExtension(command.CommandExtension):
    """
    Creates links to modal windows displaying code content.

    This extension does not add any commands; it provides a token to be created by other packages.
    It was created to have a single control point for toggling display of complete source.
    """
    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['hide_source'] = (False, "Disable all modals containing source file content.")
        config['exceptions'] = (list(), "A list of shell-style patterns for displaying certain " \
                                        "files when the 'hide_source' setting is True.")
        return config

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)

        # storage to allow all modal content to be added to end of content
        self.__modals = collections.defaultdict(list)

    def addModal(self, uid, div):
        self.__modals[uid].append(div)

    def extend(self, reader, renderer):
        renderer.add('ModalLink', RenderModalLinkToken())
        renderer.add('ModalSourceLink', RenderSourceLinkToken())

    def init(self):
        if self.get('exceptions') and not self.get('hide_source'):
            LOG.warning("The 'exceptions' setting has no effect when 'hide_source' is disabled.")

    def postRender(self, page, results):
        parent = moosetree.find(results.root, lambda n: n.name == 'div' and 'moose-content' in n['class'])
        for div in self.__modals.get(page.uid, list()):
            div.parent = parent

class RenderModalLinkToken(components.RenderComponent):
    def createHTML(self, parent, token, page):
        # Must have children, otherwise nothing exists to click
        if not token.children:
            msg = report_error("The 'ModalLink' token requires children.", page.source,
                               token.info.line if token.info else None,
                               token.info[0] if token.info else token.text())
            raise exceptions.MooseDocsException(msg)
        return html.Tag(parent, 'span', class_='moose-modal-link')

    def createMaterialize(self, parent, token, page):

        # Must have children, otherwise nothing exists to click
        if not token.children:
            msg = report_error("The 'ModalLink' token requires children.", page.source,
                               token.info.line if token.info else None,
                               token.info[0] if token.info else token.text())
            raise exceptions.MooseDocsException(msg)

        # Check 'content' is correctly provided
        content = token['content']
        if isinstance(content, str):
            content = core.Paragraph(None, string=content)

        elif not isinstance(content, tokens.Token):
            msg = report_error("The 'ModalLink' token 'content' attribute must be a string or Token.", page.source,
                               token.info.line if token.info else None,
                               token.info[0] if token.info else token.text())
            raise exceptions.MooseDocsException(msg)

        # Create the <div> for the modal content
        uid = uuid.uuid4()
        modal_div = html.Tag(None, 'div', class_='moose-modal modal', id_=str(uid))
        modal_content = html.Tag(modal_div, 'div', class_="modal-content")

        # Title
        if token['title']:
            h = core.Heading(None, level=4, string=token['title'])
            self.renderer.render(modal_content, h, page)

        # Content
        self.renderer.render(modal_content, content, page)
        self.extension.addModal(page.uid, modal_div)

        # Return link to modal window
        return html.Tag(parent, 'a', href='#{}'.format(uid), class_='moose-modal-link modal-trigger')

    def createLatex(self, parent, token, page):
        return parent

class RenderSourceLinkToken(components.RenderComponent):
    @staticmethod
    def linkText(token):
        prefix = (token['link_prefix'] + ' ') if token['link_prefix'] else ''
        path = os.path.relpath(token["src"], MooseDocs.ROOT_DIR)
        return f'({prefix}{path})' if not token.children else None

    def createHTML(self, parent, token, page):
        string = self.linkText(token)
        return html.Tag(parent, 'span', string=string, class_='moose-source-filename')

    def createMaterialize(self, parent, token, page):
        text = '({})'.format(os.path.relpath(token['src'], MooseDocs.ROOT_DIR))
        string = self.linkText(token)
        a = html.Tag(parent, 'span', string=string, class_='moose-source-filename tooltipped')

        fullpath = token['src']
        if self.extension['hide_source']:
            if not any([fnmatch.fnmatch(fullpath, p) for p in self.extension['exceptions']]):
                LOG.debug("Hide source content: %s", fullpath)
                return a

        # Create the <div> for the modal content
        uid = uuid.uuid4()
        modal_div = html.Tag(None, 'div', class_='moose-modal modal', id_=str(uid))
        modal_content = html.Tag(modal_div, 'div', class_="modal-content")
        self.extension.addModal(page.uid, modal_div)

        # Add the title and update the span to be the <a> trigger
        html.Tag(modal_content, 'h4', string=token['title'] or text)
        a.name = 'a'
        a['href'] = '#{}'.format(uid)
        a.addClass('modal-trigger')

        footer = html.Tag(modal_div, 'div', class_='modal-footer')
        html.Tag(footer, 'a', class_='modal-close btn-flat', string='Close')

        source = common.project_find(fullpath)
        if len(source) > 1:
            options = mooseutils.levenshteinDistance(fullpath, source, number=8)
            msg = "Multiple files match the supplied filename {}:\n".format(fullpath)
            for opt in options:
                msg += "    {}\n".format(opt)
            msg = report_error(msg, page.source,
                               token.info.line if token.info else None,
                               token.info[0] if token.info else token.text())
            raise exceptions.MooseDocsException(msg)

        elif len(source) == 0:
            msg = "Unable to locate file that matches the supplied filename {}\n".format(fullpath)
            msg = report_error(msg, page.source,
                               token.info.line if token.info else None,
                               token.info[0] if token.info else token.text())
            raise exceptions.MooseDocsException(msg)

        content = common.fix_moose_header(common.read(source[0]))
        language = token['language'] or common.get_language(source[0])
        code = core.Code(None, language=language, content=content)
        self.renderer.render(modal_content, code, page)

        return a

    def createLatex(self, parent, token, page):
        if not token.children:
            latex.String(parent, content=self.linkText(token))
        return parent
