#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import uuid
import collections
import anytree
import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.extensions import core
from MooseDocs.tree import tokens, html, latex

def make_extension(**kwargs):
    return FloatExtension(**kwargs)

Float = tokens.newToken('Float', img=False, bottom=False, command=u'figure')
FloatCaption = tokens.newToken('FloatCaption', key=u'', prefix=u'', number='?')
ModalLink = tokens.newToken('ModalLink', bookmark=True, bottom=False, close=True)
ModalLinkTitle = tokens.newToken('ModalLinkTitle')
ModalLinkContent = tokens.newToken('ModalLinkContent')

def create_float(parent, extension, reader, page, settings, bottom=False, img=False,
                 token_type=Float, **kwargs):
    """
    Helper for optionally creating a float based on the existence of caption and/or id.

    Inputs:
        parent: The parent token that float should be placed
        extension: The extension object (to extract 'prefix' from config items)
        reader: The Reader object for tokenization of the heading
        page: The Page object for passing to the tokenization routine
        settings: The command settings to extract a local 'prefix'
        bottom[True|False]: Set flag on the float for placing the caption at the bottom
        img[True|False]: Set to True if the contents are an image (Materialize only)
        token_type: The type of Token object to create; it should derive from Float
    """
    cap, _ = _add_caption(None, extension, reader, page, settings)
    if cap:
        flt = token_type(parent, img=img, bottom=bottom, **kwargs)
        cap.parent = flt
        return flt

    return parent

def caption_settings():
    """Return settings necessary for captions."""
    settings = dict()
    settings['caption'] = (None, "The caption text for the float object.")
    settings['prefix'] = (None, "The numbered caption label to include prior to the caption text.")
    return settings

def _add_caption(parent, extension, reader, page, settings):
    """Helper for adding captions to float tokens."""
    cap = settings['caption']
    key = settings['id']
    prefix = settings.get('prefix')
    if prefix is None:
        prefix = extension.get('prefix', None)

    if prefix is None:
        msg = "The 'prefix' must be supplied via the settings or the extension configuration."
        raise exceptions.MooseDocsException(msg)

    caption = None
    if key:
        caption = FloatCaption(parent, key=key, prefix=prefix)
        if cap:
            reader.tokenize(caption, cap, page, MooseDocs.INLINE)
    elif cap:
        caption = FloatCaption(parent)
        reader.tokenize(caption, cap, page, MooseDocs.INLINE)
    return caption, prefix

def create_modal(parent, title=None, content=None, **kwargs):
    """
    Create the necessary Modal tokens for creating modal windows with materialize.
    """
    modal = ModalLink(parent.root, **kwargs)
    if isinstance(title, str):
        ModalLinkTitle(modal, string=title)
    elif isinstance(title, tokens.Token):
        title.parent = ModalLinkTitle(modal)

    if isinstance(content, str):
        ModalLinkContent(modal, string=content)
    elif isinstance(content, tokens.Token):
        content.parent = ModalLinkContent(modal)

    return parent

def create_modal_link(parent, title=None, content=None, string=None, **kwargs):
    """
    Create the necessary tokens to create a link to a modal window with materialize.
    """
    kwargs.setdefault('bookmark', str(uuid.uuid4()))
    link = core.Link(parent,
                     url=u'#{}'.format(kwargs['bookmark']),
                     class_='modal-trigger',
                     string=string)
    create_modal(parent, title, content, **kwargs)
    return link

class FloatExtension(components.Extension):
    """
    Provides ability to add caption float elements (e.g., figures, table, etc.). This is only a
    base extension. It does not provide tables for example, just the tools to make floats
    in a uniform manner.
    """
    def extend(self, reader, renderer):
        renderer.add('Float', RenderFloat())
        renderer.add('FloatCaption', RenderFloatCaption())
        renderer.add('ModalLink', RenderModalLink())
        renderer.add('ModalLinkTitle', RenderModalLinkTitle())
        renderer.add('ModalLinkContent', RenderModalLinkContent())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('caption', labelsep='period')

    def initMetaData(self, page, meta):
        meta.initData('counts', collections.defaultdict(int))

    def postTokenize(self, ast, page, meta, reader):
        """Set float number for each counter."""
        for node in anytree.PreOrderIter(ast, filter_=lambda n: n.name == 'FloatCaption'):
            prefix = node.get('prefix', None)
            if prefix is not None:
                meta.getData('counts')[prefix] += 1
                node['number'] = meta.getData('counts')[prefix]
            key = node.get('key')
            if key:
                shortcut = core.Shortcut(ast.root, key=key, link=u'#{}'.format(key))

                # TODO: This is a bit of a hack to get Figure~\ref{} etc. working in general
                if isinstance(self.translator.renderer, LatexRenderer):
                    shortcut['prefix'] = prefix.title()
                else:
                    tokens.String(shortcut, content=u'{} {}'.format(prefix.title(), node['number']))


class RenderFloat(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', token)
        div.addClass('moose-float-div')

        if token['bottom']:
            cap = token(0)
            cap.parent = None # Guarantees that "cap" is removed from the current tree
            cap.parent = token

        return div

    def createMaterialize(self, parent, token, page): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', token)
        div.addClass('card moose-float')
        content = html.Tag(div, 'div')

        if token['img']:
            content.addClass('card-image')
        else:
            content.addClass('card-content')

        if token['bottom']:
            cap = token(0)
            cap.parent = None
            cap.parent = token

        return content

    def createLatex(self, parent, token, page):

        env = latex.Environment(parent, token['command'])
        style = latex.parse_style(token)

        width = style.get('width', None)
        if width and token(0).name == 'Image':
            token(0).set('style', 'width:{};'.format(width))

        if style.get('text-align', None) == 'center':
            latex.Command(env, 'centering')

        return env

class RenderFloatCaption(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use

        caption = html.Tag(parent, 'p', class_="moose-caption")
        prefix = token.get('prefix', None)
        if prefix:
            heading = html.Tag(caption, 'span', class_="moose-caption-heading")
            html.String(heading, content=u"{} {}: ".format(prefix, token['number']))

        return html.Tag(caption, 'span', class_="moose-caption-text")

    def createLatex(self, parent, token, page):
        caption = latex.Command(parent, 'caption')
        if token['key']:
            latex.Command(caption, 'label', string=token['key'], escape=True)
        return caption

class RenderModalLink(core.RenderLink):

    def createLatex(self, parent, token, page):
        return None

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):

        cls = "modal bottom-sheet" if token['bottom'] else "modal"
        modal = html.Tag(parent, 'div', class_=cls, id_=token['bookmark'])
        modal.addClass('moose-modal')
        modal_content = html.Tag(modal, 'div', class_="modal-content")

        if token['close']:
            footer = html.Tag(modal, 'div', class_='modal-footer')
            html.Tag(footer, 'a', class_='modal-close btn-flat', string=u'Close')
        return modal_content

class RenderModalLinkTitle(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'h4')

    def createLatex(self, parent, token, page):
        return None

class RenderModalLinkContent(components.RenderComponent):

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return parent

    def createLatex(self, parent, token, page):
        return None
