#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import logging
from MooseDocs import common, base
from MooseDocs.common import exceptions
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, media, gallery

logging.basicConfig()


class TestCard(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media, gallery]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [dict(root_dir="large_media", content=["testing/Flag_of_Idaho.svg"])]
        return common.get_content(config, ".md")

    def testAST(self):
        ast = self.tokenize("[!card!Flag_of_Idaho.svg title=Idaho](Details)")(0)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), "Card", size=3)

        self.assertToken(ast(0, 0), "CardImage", size=1)
        self.assertToken(ast(0, 0, 0), "Image", size=0, src="Flag_of_Idaho.svg")

        self.assertToken(ast(0, 1), "CardContent", size=1)
        self.assertToken(
            ast(0, 1, 0), "CardTitle", size=1, activator=True, deactivator=False
        )
        self.assertToken(ast(0, 1, 0, 0), "Word", content="Idaho")

        self.assertToken(ast(0, 2), "CardReveal", size=2)
        self.assertToken(
            ast(0, 2, 0), "CardTitle", size=1, activator=False, deactivator=True
        )
        self.assertToken(ast(0, 2, 0, 0), "Word", content="Idaho")
        self.assertToken(ast(0, 2, 1), "Paragraph", size=1)
        self.assertToken(ast(0, 2, 1, 0), "Word", content="Details")

    def testMaterialize(self):
        ast = self.tokenize("[!card!Flag_of_Idaho.svg title=Idaho](Details)")(0)
        res = self.render(ast, renderer=base.MaterializeRenderer())(0, 0)

        self.assertHTMLTag(res, "div", size=3, class_="card moose-card")
        self.assertHTMLTag(res(0), "div", size=1, class_="card-image")
        self.assertHTMLTag(res(0, 0), "picture")  # tested in test_media

        self.assertHTMLTag(res(1), "div", size=1, class_="card-content")
        self.assertHTMLTag(res(1, 0), "span", size=2, class_="card-title activator")
        self.assertHTMLString(res(1, 0, 0), "Idaho")
        self.assertHTMLTag(res(1, 0, 1), "i", string="more_vert")

        self.assertHTMLTag(res(2), "div", size=2, class_="card-reveal")
        self.assertHTMLTag(res(2, 0), "span", size=2, class_="card-title")
        self.assertHTMLString(res(2, 0, 0), "Idaho")
        self.assertHTMLTag(res(2, 0, 1), "i", string="close")
        self.assertHTMLTag(res(2, 1), "p", string="Details")


class TestSlideshow(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media, gallery]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [
            dict(
                root_dir="large_media",
                content=[
                    "testing/Flag_of_Idaho.svg",
                    "testing/Flag_of_Washington.svg",
                ],
            )
        ]
        return common.get_content(config, ".md")

    SLIDESHOW = (
        "!slideshow! interval=3\n"
        "!slide Flag_of_Idaho.svg caption=Idaho\n\n"
        "!slide Flag_of_Washington.svg\n"
        "!slideshow-end!"
    )

    def testAST(self):
        ast = self.tokenize(self.SLIDESHOW)(0)
        self.assertToken(ast, "Slideshow", size=2, interval=3.0, overlay_default=False)

        self.assertToken(ast(0), "Slide", size=2, overlay=None)
        self.assertToken(ast(0, 0), "Image", size=0, src="Flag_of_Idaho.svg")
        self.assertToken(ast(0, 1), "SlideContent", size=1)
        self.assertToken(ast(0, 1, 0), "SlideCaption", size=1)
        self.assertToken(ast(0, 1, 0, 0), "Word", content="Idaho")

        self.assertToken(ast(1), "Slide", size=1)
        self.assertToken(ast(1, 0), "Image", size=0, src="Flag_of_Washington.svg")

    def testMaterialize(self):
        ast = self.tokenize(self.SLIDESHOW)(0)
        res = self.render(ast, renderer=base.MaterializeRenderer())(0)

        self.assertHTMLTag(
            res, "div", size=2, class_="carousel carousel-slider moose-slideshow"
        )
        self.assertEqual(res["data-interval"], "3000")

        self.assertHTMLTag(res(0), "div", class_="carousel-item")
        self.assertHTMLTag(res(0, 0, 0), "picture")  # image, tested in test_media
        self.assertHTMLTag(res(0, 0, 1), "div", size=1, class_="moose-slide-content")
        self.assertHTMLTag(res(0, 0, 1, 0), "span", size=1, class_="carousel-caption")
        self.assertHTMLString(res(0, 0, 1, 0, 0), "Idaho")

        self.assertHTMLTag(res(1), "div", size=1, class_="carousel-item")
        self.assertHTMLTag(res(1, 0), "div", size=1, class_="moose-slide")
        self.assertHTMLTag(res(1, 0, 0), "picture")

    # A slide's 'overlay' setting overrides the slideshow's 'overlay_default'.
    SLIDESHOW_OVERLAY = (
        "!slideshow! overlay_default=True\n"
        "!slide Flag_of_Idaho.svg caption=Idaho\n\n"
        "!slide Flag_of_Washington.svg caption=Washington overlay=False\n"
        "!slideshow-end!"
    )

    def testOverlay(self):
        ast = self.tokenize(self.SLIDESHOW_OVERLAY)(0)
        self.assertToken(ast, "Slideshow", size=2, overlay_default=True)
        self.assertToken(ast(0), "Slide", overlay=None)
        self.assertToken(ast(1), "Slide", overlay=False)

        res = self.render(ast, renderer=base.MaterializeRenderer())(0)
        self.assertHTMLTag(
            res(0, 0, 1), "div", class_="moose-slide-content moose-slide-overlay"
        )
        self.assertHTMLTag(res(1, 0, 1), "div", class_="moose-slide-content")

    # The paired form carries arbitrary block content (e.g. a link to the relevant model)
    # beneath the image and caption.
    SLIDESHOW_CONTENT = (
        "!slideshow!\n"
        "!slide! Flag_of_Idaho.svg caption=Idaho\n"
        "[model](https://example.com)\n"
        "!slide-end!\n"
        "!slideshow-end!"
    )

    def testSlideContent(self):
        ast = self.tokenize(self.SLIDESHOW_CONTENT)(0)
        self.assertToken(ast, "Slideshow", size=1)

        self.assertToken(ast(0), "Slide", size=2)
        self.assertToken(ast(0, 0), "Image", size=0, src="Flag_of_Idaho.svg")
        self.assertToken(ast(0, 1), "SlideContent", size=2)
        self.assertToken(ast(0, 1, 0), "SlideCaption", size=1)
        self.assertToken(ast(0, 1, 0, 0), "Word", content="Idaho")
        self.assertToken(ast(0, 1, 1), "Paragraph", size=2)  # Link + trailing Break
        self.assertToken(ast(0, 1, 1, 0), "Link", url="https://example.com")

        res = self.render(ast, renderer=base.MaterializeRenderer())(0)
        self.assertHTMLTag(res(0), "div", class_="carousel-item")
        self.assertHTMLTag(res(0, 0, 0), "picture")
        self.assertHTMLTag(res(0, 0, 1), "div", size=2, class_="moose-slide-content")
        self.assertHTMLTag(res(0, 0, 1, 0), "span", size=1, class_="carousel-caption")
        self.assertHTMLTag(res(0, 0, 1, 1), "p")
        self.assertHTMLTag(res(0, 0, 1, 1, 0), "a")


if __name__ == "__main__":
    unittest.main(verbosity=2)
