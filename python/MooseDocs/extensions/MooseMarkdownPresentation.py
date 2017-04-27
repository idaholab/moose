import os
import markdown
import logging
log = logging.getLogger(__name__)

from MooseSlidePreprocessor import MooseSlidePreprocessor

class MooseMarkdownPresentation(markdown.Extension):
    def __init__(self, *args, **kwargs):
        self.config = dict()
        super(MooseMarkdownPresentation, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):
        """
        Builds the extensions for MOOSE Presentation Slides, which uses Reveal.js.
        """
        md.registerExtension(self)
        md.preprocessors.add('moose_slides', MooseSlidePreprocessor(markdown_instance=md), '_end')

def makeExtension(*args, **kwargs):
    return MooseMarkdownPresentation(*args, **kwargs)
