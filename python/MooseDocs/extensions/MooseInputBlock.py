import os
import MooseDocs
from MooseTextPatternBase import MooseTextPatternBase
from FactorySystem import ParseGetPot

class MooseInputBlock(MooseTextPatternBase):
    """
    Markdown extension for extracting blocks from input files.
    """

    CPP_RE = r'!input\s+(.*\/(.*?))\s+(.*)'

    def __init__(self, src):
        super(MooseInputBlock, self).__init__(self.CPP_RE, src, 'text')

    def handleMatch(self, match):
        """
        Process the input file supplied.
        """

        # Update the settings from regex match
        settings = self.getSettings(match.group(4))

        # Build the complete filename.
        # NOTE: os.path.join doesn't like the unicode even if you call str() on it first.
        rel_filename = match.group(2).lstrip('/')
        filename = MooseDocs.MOOSE_DIR.rstrip('/') + os.path.sep + rel_filename

        # Read the file and create element
        filename = self.checkFilename(rel_filename)
        if not filename:
            el = self.createErrorElement(rel_filename)
        elif not settings.has_key('block'):
            el = self.createErrorElement(rel_filename, message="Use of !input syntax while not providing a block=some_block. If you wish to include the entire file, use !text instead")
        else:
            parser = ParseGetPot(filename)
            node = parser.root_node.getNode(settings['block'])

            if node == None:
                content = 'ERROR: Failed to find {} in {}.'.format(match.group(3), rel_filename)
            else:
                content = node.createString()

            if match.group(2):
                label = match.group(2)
            else:
                label = rel_filename
            el = self.createElement(label, content, filename, rel_filename, settings)

        return el
