import os
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension

class MooseFigureReference(MooseCommonExtension, Pattern):
    """
    Defines syntax for referencing figures.
    """

    RE = r'(?<!`)\\ref{(.*?)}'

    def __init__(self, markdown_instance=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

    def handleMatch(self, match):
        el = etree.Element('a')
        el.set('class', 'moose-figure-reference')
        el.set('href', '#' + match.group(2))
        return el
