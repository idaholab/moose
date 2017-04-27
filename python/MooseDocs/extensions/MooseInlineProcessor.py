import logging
log = logging.getLogger(__name__)
from markdown.treeprocessors import InlineProcessor
from MooseImageFile import MooseImageFile
from MooseCommonExtension import MooseCommonExtension

class MooseInlineProcessor(InlineProcessor, MooseCommonExtension):
    """
    Replacement for the standard InlineProcessor that includes special features for MooseMarkdown.
        1. Calls an initialize() method on all inline patterns before processing the tree
    """
    def __init__(self, markdown_instance=None, **kargs):
        super(MooseInlineProcessor, self).__init__(markdown_instance)

    def run(self, root, **kwargs):
        """
        Adds a call to 'initialize()' prior to executing the InlineProcessor running.
        """

        # Call initialize method (see MooseFigure)
        for inline in self.inlinePatterns.itervalues():
            if hasattr(inline, 'initialize'):
                inline.initialize()

        super(MooseInlineProcessor, self).run(root, **kwargs)
