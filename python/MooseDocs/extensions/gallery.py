#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import logging
from ..common import exceptions
from ..base import components, LatexRenderer
from ..tree import tokens, html, latex
from . import command, core, media

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return GalleryExtension(**kwargs)

CARD_LATEX = """\\NewDocumentEnvironment{card}{mm}{ %
  \\tcbset{width=#1,title=#2}
  \\begin{tcolorbox}[fonttitle=\\bfseries, colback=white, colframe=card-frame]
}{ %
  \\end{tcolorbox}
}
"""

Card = tokens.newToken('Card')
CardImage = tokens.newToken('CardImage')
CardContent = tokens.newToken('CardContent')
CardReveal = tokens.newToken('CardReveal')
CardTitle = tokens.newToken('CardTitle', deactivator=False, activator=False)
Gallery = tokens.newToken('Gallery', large=3, medium=6, small=12)

class GalleryExtension(command.CommandExtension):
    """
    Adds commands needed to create image galleries.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(core, command, media)
        self.addCommand(reader, CardComponent())
        self.addCommand(reader, GalleryComponent())
        renderer.add('Card', RenderCard())
        renderer.add('CardImage', RenderCardImage())
        renderer.add('CardContent', RenderCardContent())
        renderer.add('CardReveal', RenderCardReveal())
        renderer.add('CardTitle', RenderCardTitle())
        renderer.add('Gallery', RenderGallery())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('tcolorbox')
            renderer.addPackage('xparse')
            renderer.addPreamble('\\definecolor{card-frame}{RGB}{0,88,151}')
            renderer.addPreamble(CARD_LATEX)

class CardComponent(command.CommandComponent):
    COMMAND = 'card'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg', 'ogg', 'webm', 'mp4')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['title'] = (None, "Title of the card.")
        return settings

    def createToken(self, parent, info, page, settings):
        card = Card(parent, **self.attributes(settings))

        # Insert image or movie
        img = CardImage(card)
        src = info['subcommand']
        if src.endswith(('.ogg', '.webm', '.mp4')):
            media.Video(img, src=src, class_='activator')
        else:
            media.Image(img, src=src, class_='activator')

        # A title is required
        title = settings['title']
        if title is None:
            raise exceptions.MooseDocsException("The 'title' option is required.")

        # Content (the title when the card is not showing the detailed content)
        card_content = CardContent(card)
        card_title = CardTitle(card_content)
        self.reader.tokenize(card_title, title, page, 'inline', line=info.line)

        # Detailed content
        reveal = info['block'] if 'block' in info else info['inline']
        if reveal:
            card_reveal = CardReveal(card) # contains the detailed content

            # Add the title to the detailed content, with a close (deactivator) button
            reveal_title = card_title.copy()
            reveal_title['deactivator'] = True
            reveal_title.parent = card_reveal

            # Tokenize the content within the card
            self.reader.tokenize(card_reveal, reveal, page, line=info.line)

            # The main title will activate the reveal
            card_title['activator'] = True

        return parent

class GalleryComponent(command.CommandComponent):
    COMMAND = 'gallery'
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['large'] = (4, "Number of columns on large screens (1-12).")
        settings['medium'] = (6, "Number of columns on medium screens (1-12).")
        settings['small'] = (12, "Number of columns on small screens (1-12).")
        return settings

    def createToken(self, parent, info, page, settings):
        return Gallery(parent,
                       large=int(settings['large']),
                       medium=int(settings['medium']),
                       small=int(settings['small']))

class RenderCard(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        if token.parent.name == 'Gallery':
            class_ = 'col s{} m{} l{}'.format(token.parent['small'],
                                              token.parent['medium'],
                                              token.parent['large'])
            parent = html.Tag(parent, 'div', class_=class_)

        div = html.Tag(parent, 'div', token)
        div.addClass('card')
        div.addClass('moose-card')
        return div

    def createLatex(self, parent, token, page):

        args = []
        style = latex.parse_style(token)
        width = style.get('width', None)
        if width:
            if width.endswith('%'):
                width = '{}\\textwidth'.format(int(width[:-1])/100.)
            args.append(latex.Brace(string=width, escape=False))
            token.children[0]['width'] = width
        else:
            args.append(latex.Brace(string='\\textwidth', escape=False))

        if (len(token.children) > 1) and (token.children[1].name == 'CardContent'):
            title = latex.Brace()
            self.translator.renderer.render(title, token.children[1], page)
            token.children[1].parent = None
            args.append(title)

        return latex.Environment(latex.Environment(parent, 'center'), 'card', args=args)

class RenderCardImage(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return parent

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'div', class_='card-image')

    def createLatex(self, parent, token, page):
        return parent

class RenderCardContent(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'div', class_='card-content')

class RenderCardReveal(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, 'div', class_='card-reveal')

class RenderCardTitle(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        span = html.Tag(parent, 'span', class_='card-title')
        for child in token:
            self.renderer.render(span, child, page)
        if token['activator']:
            span.addClass('activator')
            html.Tag(span, 'i', class_='material-icons right', string='more_vert')
        elif token['deactivator']:
            html.Tag(span, 'i', class_='material-icons right', string='close')
        return None

class RenderGallery(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        for child in token.children:
            if child.name != 'Card':
                msg = "The 'gallery' command requires that all content be within cards (i.e., " \
                      "created with the 'card' command. However, one of the children of the " \
                      "gallery is a '%s' token."
                LOG.error(msg, child.name)

        row = html.Tag(parent, 'div', token)
        row.addClass('row')
        return row
