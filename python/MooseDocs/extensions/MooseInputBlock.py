import os
import MooseDocs
from MooseSourcePatternBase import MooseSourcePatternBase
from FactorySystem import ParseGetPot

class MooseInputBlock(MooseSourcePatternBase):
    """
    Markdown extension for extracting blocks from input files.
    """

    CPP_RE = r'!\[(.*?)\]\((.*\.[i])::(\w+)\s*(.*?)\)'

    def __init__(self, src):
        super(MooseInputBlock, self).__init__(self.CPP_RE, src, 'text')

    def handleMatch(self, match):
        """
        Process the input file supplied.
        """

        # Update the settings from regex match
        settings = self.getSettings(match.group(5))

        # Build the complete filename.
        # NOTE: os.path.join doesn't like the unicode even if you call str() on it first.
        rel_filename = match.group(3).lstrip('/')
        filename = MooseDocs.MOOSE_DIR.rstrip('/') + os.path.sep + rel_filename

        # Read the file and create element
        filename = self.checkFilename(rel_filename)
        if not filename:
            el = self.createErrorElement(rel_filename)
        else:
            parser = ParseGetPot(filename)
            node = parser.root_node.getNode(match.group(4))

            if node == None:
                content = 'ERROR: Failed to find {} in {}.'.format(match.group(2), rel_filename)
            else:
                content = node.createString()

            label = '{} [{}]'.format(match.group(2), match.group(4))
            el = self.createElement(label, content, filename, rel_filename, settings)

        return el
