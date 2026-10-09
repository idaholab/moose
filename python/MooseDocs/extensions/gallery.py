# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import logging
from ..common import exceptions
from ..base import components, LatexRenderer, MaterializeRenderer
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

Card = tokens.newToken("Card")
CardImage = tokens.newToken("CardImage")
CardContent = tokens.newToken("CardContent")
CardReveal = tokens.newToken("CardReveal")
CardTitle = tokens.newToken("CardTitle", deactivator=False, activator=False)
Gallery = tokens.newToken("Gallery", large=3, medium=6, small=12)
Slideshow = tokens.newToken("Slideshow", interval=None, overlay_default=False)
Slide = tokens.newToken("Slide", overlay=None)
SlideContent = tokens.newToken("SlideContent")
SlideCaption = tokens.newToken("SlideCaption")


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
        self.addCommand(reader, SlideshowComponent())
        self.addCommand(reader, SlideComponent())
        renderer.add("Card", RenderCard())
        renderer.add("CardImage", RenderCardImage())
        renderer.add("CardContent", RenderCardContent())
        renderer.add("CardReveal", RenderCardReveal())
        renderer.add("CardTitle", RenderCardTitle())
        renderer.add("Gallery", RenderGallery())
        renderer.add("Slideshow", RenderSlideshow())
        renderer.add("Slide", RenderSlide())
        renderer.add("SlideContent", RenderSlideContent())
        renderer.add("SlideCaption", RenderSlideCaption())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage("tcolorbox")
            renderer.addPackage("xparse")
            renderer.addPreamble("\\definecolor{card-frame}{RGB}{0,88,151}")
            renderer.addPreamble(CARD_LATEX)

        # The slideshow relies on the Materialize carousel component; register the
        # initialization script that wires up autoplay, which Materialize does not provide
        # on its own.
        if isinstance(renderer, MaterializeRenderer):
            renderer.addJavaScript("moose_slideshow", "js/carousel.js")


class CardComponent(command.CommandComponent):
    COMMAND = "card"
    SUBCOMMAND = ("jpg", "jpeg", "gif", "png", "svg", "ogg", "webm", "mp4")

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["title"] = (None, "Title of the card.")
        return settings

    def createToken(self, parent, info, page, settings):
        card = Card(parent, **self.attributes(settings))

        # Insert image or movie
        img = CardImage(card)
        src = info["subcommand"]
        if src.endswith((".ogg", ".webm", ".mp4")):
            media.Video(img, src=src, class_="activator", alt=settings["title"])
        else:
            media.Image(img, src=src, class_="activator", alt=settings["title"])

        # A title is required
        title = settings["title"]
        if title is None:
            raise exceptions.MooseDocsException("The 'title' option is required.")

        # Content (the title when the card is not showing the detailed content)
        card_content = CardContent(card)
        card_title = CardTitle(card_content)
        self.reader.tokenize(card_title, title, page, "inline", line=info.line)

        # Detailed content
        reveal = info["block"] if "block" in info else info["inline"]
        if reveal:
            card_reveal = CardReveal(card)  # contains the detailed content

            # Add the title to the detailed content, with a close (deactivator) button
            reveal_title = card_title.copy()
            reveal_title["deactivator"] = True
            reveal_title.parent = card_reveal

            # Tokenize the content within the card
            self.reader.tokenize(card_reveal, reveal, page, line=info.line)

            # The main title will activate the reveal
            card_title["activator"] = True

        return parent


class GalleryComponent(command.CommandComponent):
    COMMAND = "gallery"
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["large"] = (4, "Number of columns on large screens (1-12).")
        settings["medium"] = (6, "Number of columns on medium screens (1-12).")
        settings["small"] = (12, "Number of columns on small screens (1-12).")
        return settings

    def createToken(self, parent, info, page, settings):
        return Gallery(
            parent,
            large=int(settings["large"]),
            medium=int(settings["medium"]),
            small=int(settings["small"]),
        )


class SlideshowComponent(command.CommandComponent):
    COMMAND = "slideshow"
    SUBCOMMAND = None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["interval"] = (
            None,
            "Auto-advance period in seconds; if unset, slides advance only when the "
            "viewer clicks a control.",
        )
        settings["overlay_default"] = (
            False,
            "Default for the 'overlay' setting of each slide in the slideshow.",
        )
        return settings

    def createToken(self, parent, info, page, settings):
        return Slideshow(
            parent,
            interval=settings["interval"],
            overlay_default=settings["overlay_default"],
            **self.attributes(settings),
        )


class SlideComponent(command.CommandComponent):
    COMMAND = "slide"
    SUBCOMMAND = ("jpg", "jpeg", "gif", "png", "svg")

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings["caption"] = (None, "Caption text displayed beneath the slide image.")
        settings["dark_src"] = (None, "Image to utilize with dark HTML theme.")
        settings["alt"] = (
            None,
            "Alt text describing the image (defaults to the caption).",
        )
        settings["overlay"] = (
            None,
            "Overlay the caption and any block content on the bottom of the image rather than "
            "placing them beneath it; defaults to the slideshow's 'overlay_default' setting.",
        )
        return settings

    def createToken(self, parent, info, page, settings):
        # The paired form (!slide!...!slide-end!) carries arbitrary block content (e.g. a
        # caption line plus a link to the relevant model) in the regex 'block' group, which
        # the recursive lexer tokenizes into the returned token. The caption and the block
        # content share one SlideContent token, so the 'overlay' setting places them together;
        # any 'caption' setting is rendered first, so the block content follows beneath it.
        slide = Slide(parent, overlay=settings["overlay"], **self.attributes(settings))
        media.Image(
            slide,
            src=info["subcommand"],
            dark=settings["dark_src"],
            alt=settings["alt"] or settings["caption"],
        )
        block = info["block"] if "block" in info else None
        if not (settings["caption"] or block):
            return slide

        content = SlideContent(slide)
        if settings["caption"]:
            caption = SlideCaption(content)
            self.reader.tokenize(
                caption, settings["caption"], page, "inline", line=info.line
            )
        return content


class RenderCard(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        if token.parent.name == "Gallery":
            class_ = "col s{} m{} l{}".format(
                token.parent["small"], token.parent["medium"], token.parent["large"]
            )
            parent = html.Tag(parent, "div", class_=class_)

        div = html.Tag(parent, "div", token)
        div.addClass("card")
        div.addClass("moose-card")
        return div

    def createLatex(self, parent, token, page):

        args = []
        style = latex.parse_style(token)
        width = style.get("width", None)
        if width:
            if width.endswith("%"):
                width = "{}\\textwidth".format(int(width[:-1]) / 100.0)
            args.append(latex.Brace(string=width, escape=False))
            token.children[0]["width"] = width
        else:
            args.append(latex.Brace(string="\\textwidth", escape=False))

        if (len(token.children) > 1) and (token.children[1].name == "CardContent"):
            title = latex.Brace()
            self.translator.renderer.render(title, token.children[1], page)
            token.children[1].parent = None
            args.append(title)

        return latex.Environment(latex.Environment(parent, "center"), "card", args=args)


class RenderCardImage(components.RenderComponent):
    def createHTML(self, parent, token, page):
        return parent

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, "div", class_="card-image")

    def createLatex(self, parent, token, page):
        return parent


class RenderCardContent(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, "div", class_="card-content")


class RenderCardReveal(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, "div", class_="card-reveal")


class RenderCardTitle(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        span = html.Tag(parent, "span", class_="card-title")
        for child in token:
            self.renderer.render(span, child, page)
        if token["activator"]:
            span.addClass("activator")
            html.Tag(span, "i", class_="material-icons right", string="more_vert")
        elif token["deactivator"]:
            html.Tag(span, "i", class_="material-icons right", string="close")
        return None


class RenderGallery(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return None

    def createMaterialize(self, parent, token, page):
        for child in token.children:
            if child.name != "Card":
                msg = (
                    "The 'gallery' command requires that all content be within cards (i.e., "
                    "created with the 'card' command). However, one of the children of the "
                    "gallery is a '%s' token."
                )
                LOG.error(msg, child.name)

        row = html.Tag(parent, "div", token)
        row.addClass("row")
        return row


class RenderSlideshow(components.RenderComponent):
    def createLatex(self, parent, token, page):
        # No carousel in LaTeX; the slide images render in order (see RenderSlide).
        return parent

    def createHTML(self, parent, token, page):
        # Without the Materialize carousel (plain HTML, PDF), degrade to the slide images
        # in order rather than dropping them, since the images are the content.
        return parent

    def createMaterialize(self, parent, token, page):
        for child in token.children:
            if child.name != "Slide":
                msg = (
                    "The 'slideshow' command requires that all content be slides (i.e., "
                    "created with the 'slide' command). However, one of the children of the "
                    "slideshow is a '%s' token."
                )
                LOG.error(msg, child.name)

        div = html.Tag(parent, "div", token)
        div.addClass("carousel")
        # 'carousel-slider' selects Materialize's full-width layout (one 100%-wide slide at a
        # time), which pairs with the fullWidth option the init script passes. Without it each
        # slide is a fixed 200px box and neighboring slides remain visible beside the active one.
        div.addClass("carousel-slider")
        div.addClass("moose-slideshow")
        if token["interval"]:
            # The carousel script uses milliseconds, as does JavaScript's setInterval.
            div["data-interval"] = str(round(token["interval"] * 1000))
        return div


class RenderSlide(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return parent

    def createMaterialize(self, parent, token, page):
        # A <div> (not an <a>) so that a slide can hold arbitrary content, including links:
        # a link inside an <a class="carousel-item"> would be an invalid nested anchor, which
        # the browser splits out of the slide.
        div = html.Tag(parent, "div", token)
        div.addClass("carousel-item")
        # The carousel item spans the full carousel width, so the image and its content sit in
        # an inner box as wide as the image, which bounds an overlay to the image.
        return html.Tag(div, "div", class_="moose-slide")


class RenderSlideContent(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return parent

    def createMaterialize(self, parent, token, page):
        slide = token.parent
        overlay = slide["overlay"]
        if overlay is None:
            overlay = slide.parent.get("overlay_default", False)

        div = html.Tag(parent, "div", token)
        div.addClass("moose-slide-content")
        if overlay:
            div.addClass("moose-slide-overlay")
        return div


class RenderSlideCaption(components.RenderComponent):
    def createLatex(self, parent, token, page):
        return parent

    def createHTML(self, parent, token, page):
        return html.Tag(parent, "p", class_="moose-caption")

    def createMaterialize(self, parent, token, page):
        return html.Tag(parent, "span", class_="carousel-caption")
