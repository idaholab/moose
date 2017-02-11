import os
import markdown
import logging
log = logging.getLogger(__name__)

from MooseSlidePreprocessor import MooseSlidePreprocessor
from MooseSlideContents import MooseSlideContents

class MooseMarkdownPresentationExtension(markdown.Extension):
    def __init__(self, *args, **kwargs):
        self.config = dict()
        super(MooseMarkdownPresentationExtension, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE Presentation Slides, which uses Reveal.js.
        """
        md.registerExtension(self)
        md.preprocessors.add('moose_slides', MooseSlidePreprocessor(markdown_instance=md), '_end')

        md.postprocessors.add('moose_slide_contents', MooseSlideContents(markdown_instance=md), '<moose_template')


def makeExtension(*args, **kwargs):
    return MooseMarkdownPresentationExtension(*args, **kwargs)
