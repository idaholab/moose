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
import re
import logging
import moosetree
import MooseDocs
from ..common import exceptions, report_error
from ..base import components, MarkdownReader, LatexRenderer, Extension
from ..tree import tokens, html, latex
from . import core, command, heading

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return FloatExtension(**kwargs)

Float = tokens.newToken('Float', img=False, bottom=False, command='figure')
FloatCaption = tokens.newToken('FloatCaption', key='', prefix='', number='?')
FloatReference = tokens.newToken('FloatReference', label=None, filename=None)

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
            reader.tokenize(caption, cap, page, MarkdownReader.INLINE)
    elif cap:
        caption = FloatCaption(parent)
        reader.tokenize(caption, cap, page, MarkdownReader.INLINE)
    return caption, prefix

class FloatExtension(command.CommandExtension):
    """
    Provides ability to add caption float elements (e.g., figures, table, etc.). This is only a
    base extension. It does not provide tables for example, just the tools to make floats
    in a uniform manner.
    """
    def extend(self, reader, renderer):
        self.requires(core)
        self.addCommand(reader, FloatReferenceCommand())
        renderer.add('Float', RenderFloat())
        renderer.add('FloatCaption', RenderFloatCaption())
        renderer.add('FloatReference', RenderFloatReference())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('caption', labelsep='period')

    def postTokenize(self, page, ast):
        """Set float number for each counter."""
        counts = collections.defaultdict(int)
        floats = dict()
        for node in moosetree.iterate(ast, lambda n: n.name == 'FloatCaption'):
            prefix = node.get('prefix', None)
            if prefix is not None:
                counts[prefix] += 1
                node['number'] = counts[prefix]
            key = node.get('key')
            if key:
                floats[key] = node.copy()
                shortcut = core.Shortcut(ast.root, key=key, link='#{}'.format(key))

                # TODO: This is a bit of a hack to get Figure~\ref{} etc. working in general
                if isinstance(self.translator.renderer, LatexRenderer):
                    shortcut['prefix'] = prefix.title()
                else:
                    tokens.String(shortcut, content='{} {}'.format(prefix.title(), node['number']))

        page['counts'] = counts
        page['floats'] = floats

class FloatReferenceCommand(command.CommandComponent):
    COMMAND = 'ref'
    SUBCOMMAND = None
    LABEL_RE = re.compile(r'((?P<filename>.*?\.md)#)?(?P<label>.+)', flags=re.UNICODE)

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        inline = 'inline' in info
        if not inline:
            raise common.exceptions.MooseDocsException("The float reference command is an inline level command.")

        content = info['inline']
        match = self.LABEL_RE.search(content)
        if match is None:
            raise common.exceptions.MooseDocsException("Invalid label format.")

        FloatReference(parent, label=match.group('label'), filename=match.group('filename'))
        return parent

class RenderFloat(components.RenderComponent):
    def createHTML(self, parent, token, page):
        div = html.Tag(parent, 'div', token)
        div.addClass('moose-float-div')

        if token['bottom']:
            cap = token(0)
            cap.parent = None # Guarantees that "cap" is removed from the current tree
            cap.parent = token

        return div

    def createMaterialize(self, parent, token, page):
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
        if width and token(1).name == 'Image':
            token(1)['style'] = 'width:{};'.format(width)

        if style.get('text-align', None) == 'center':
            latex.Command(env, 'centering')

        if token['bottom']:
            cap = token(0)
            cap.parent = None
            cap.parent = token

        return env

class RenderFloatCaption(components.RenderComponent):
    def createHTML(self, parent, token, page):

        caption = html.Tag(parent, 'p', class_="moose-caption")
        prefix = token.get('prefix', None)
        if prefix:
            heading = html.Tag(caption, 'span', class_="moose-caption-heading")
            html.String(heading, content="{} {}: ".format(prefix, token['number']))

        return html.Tag(caption, 'span', class_="moose-caption-text", id_=token['key'])

    def createLatex(self, parent, token, page):
        caption = latex.Command(parent, 'caption')
        if token['key']:
            latex.Command(caption, 'label', string=token['key'], escape=False)
        return caption

class RenderFloatReference(core.RenderShortcutLink):
    def createHTML(self, parent, token, page):
        a = html.Tag(parent, 'a', class_='moose-float-reference')

        float_page = page
        if token['filename']:
            float_page = self.translator.findPage(token['filename'], throw_on_zero=False)
            if float_page is None:
                a['class'] = 'moose-error'
                html.String(a, content='{}#{}'.format(token['filename'], key))
                msg = "Could not find  page {}".format(token['filename'])
                raise exceptions.MooseDocsException(msg)
                return None

            head = heading.find_heading(float_page)
            if head is not None:
                tok = tokens.Token(None)
                head.copyToToken(tok)
                self.renderer.render(a, tok, page)
                html.String(a, content=', ')
            else:
                html.String(a, content=token['filename'] + ', ')

        key = token['label']
        float_node = float_page['floats'].get(key, None)
        if float_node is None:
            a['class'] = 'moose-error'
            html.String(a, content='{}#{}'.format(float_page.local, key))
            msg = "Could not find float with key {} on page {}".format(key, float_page.local)
            raise exceptions.MooseDocsException(msg)
            return None
        elif float_page is not page:
            url = float_page.relativeDestination(page)
            a['href']='{}#{}'.format(url, key)
        else:
            a['href']='#{}'.format(key)

        prefix = float_node.get('prefix', None)
        prefix = '' if prefix is None else prefix.title()
        html.String(a, content='{} {}'.format(prefix, float_node['number']))

    def createLatex(self, parent, token, page):
        float_page = page
        if token['filename']:
            float_page = self.translator.findPage(token['filename'], throw_on_zero=False)
        key = token['label']
        float_node = float_page['floats'].get(key, None)
        prefix = float_node.get('prefix', None)
        prefix = '' if prefix is None else prefix.title()
        latex.String(parent, content=prefix + '~', escape=False)
        latex.Command(parent, 'ref', string=token['label'], escape=False)
        return parent
