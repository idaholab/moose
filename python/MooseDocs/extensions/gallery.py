#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
from MooseDocs.base import components
from MooseDocs.extensions import command, core
from MooseDocs.tree import tokens, html
LOG = logging.getLogger(__name__)
def make_extension(**kwargs):
    return GalleryExtension(**kwargs)


Card = tokens.newToken('Card')
CardImage = tokens.newToken('CardImage', src=u'')
CardTitle = tokens.newToken('CardTitle')
CardContent = tokens.newToken('CardContent')
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
        self.requires(core, command)
        self.addCommand(reader, CardComponent())
        self.addCommand(reader, GalleryComponent())
        renderer.add('Card', RenderCard())
        renderer.add('CardImage', RenderCardImage())
        renderer.add('CardTitle', RenderCardTitle())
        renderer.add('CardContent', RenderCardContent())
        renderer.add('Gallery', RenderGallery())

class CardComponent(command.CommandComponent):
    COMMAND = 'card'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg', 'ogg', 'webm', 'mp4')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['title'] = (u'', "Title of the card.")
        return settings

    def createToken(self, parent, info, page):

        card = Card(parent, **self.attributes)

        src = info['subcommand']
        if src.startswith('http'):
            location = src
        else:
            node = self.translator.findPage(src)
            location = unicode(node.relativeSource(page))
        CardImage(card, src=location)

        if self.settings['title']:
            card_title = CardTitle(card)
            self.reader.tokenize(card_title, self.settings['title'], page, 'inline', line=info.line)

        content = info['block'] if 'block' in info else info['inline']
        if content:
            card_content = CardContent(card)
            self.reader.tokenize(card_content, content, page, line=info.line)

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

    def createToken(self, parent, info, page):
        return Gallery(parent,
                       large=self.settings['large'],
                       medium=self.settings['medium'],
                       small=self.settings['small'])

class RenderCard(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return None

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

class RenderCardImage(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return None

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        div = html.Tag(parent, 'div', class_='card-image')
        html.Tag(div, 'img', class_='activator', src=token['src'])

class RenderCardTitle(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return None

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        div = html.Tag(parent, 'div', class_='card-content')
        span = html.Tag(div, 'span', class_='card-title activator grey-text text-darken-4')
        if token.next:
            html.Tag(span, 'i', class_='material-icons right', string='more_vert')
        return span

class RenderCardContent(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return None

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        div = html.Tag(parent, 'div', class_='card-reveal')
        title = div.previous
        if title is not None:
            title = title.copy()
            title.parent = div
            title(0)(0)(0)['content'] = 'close'
        return div

class RenderGallery(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return None

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
