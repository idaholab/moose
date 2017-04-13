from markdown.treeprocessors import InlineProcessor
import logging
log = logging.getLogger(__name__)

from MooseImageFile import MooseImageFile
from MooseCommonExtension import MooseCommonExtension

class MooseInlineProcessor(InlineProcessor, MooseCommonExtension):
    """
    Replacement for the standard InlineProcessor that includes an initialize() method.
    """

    def __init__(self, markdown_instance=None, **kwargs):
        super(MooseInlineProcessor, self).__init__(markdown_instance)

    def run(self, *args, **kwargs):
        """
        Adds a call to 'initialize()' prior to executing the InlineProcessor running.
        """
        for inline in self.inlinePatterns.itervalues():
            if hasattr(inline, 'initialize'):
                inline.initialize()
        super(MooseInlineProcessor, self).run(*args, **kwargs)
