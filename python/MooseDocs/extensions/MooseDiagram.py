import os
import subprocess
import re
import uuid
from markdown.blockprocessors import BlockProcessor
from MooseCommonExtension import MooseCommonExtension
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

class MooseDiagram(BlockProcessor, MooseCommonExtension):
    """
    Extension to allow for dot diagrams.
    """

    RE = re.compile(r'^(graph|dirgraph)(.*)')

    def __init__(self, md, graphviz=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        BlockProcessor.__init__(self, md)

        # Location of the graphviz
        self._graphviz = graphviz

    def test(self, parent, block):
        """
        Test if the block contains the diagram keywords.
        """
        return self.RE.search(block)

    def run(self, parent, blocks):
        """
        Generate dot svg and html for displaying the image.
        """

        # Extract the block of interest
        block = blocks.pop(0)

        # Test if graphviz can be found
        executable = os.path.join(self._graphviz, 'dot')
        if not os.path.exists(executable):
            return self.createErrorElement(block, title='Failed to locate Graphviz', parent=parent)

        # Create the temporary dot file
        dot_file = 'tmp_' + uuid.uuid4().hex + '.dot'
        with open(dot_file, 'w') as fid:
            fid.write(block)

        # Create a temporary svg file
        svg_file = 'media/tmp_' + uuid.uuid4().hex + '.moose.svg'

        # Execute graphviz to generate the svg file
        try:
            cmd = [executable, '-Tsvg', dot_file, '-o', svg_file]
            output = subprocess.check_output(cmd)
            log.debug('Created SVG chart using dot: {}'.format(svg_file))
        except:
            if os.path.exists(dot_file):
                os.remove(dot_file)
            return self.createErrorElement(block, title='Failed to execute Graphviz', parent=parent)

        # Clean up dot temporary
        if os.path.exists(dot_file):
            os.remove(dot_file)

        # Create the img that will contain the flow chart
        img = etree.SubElement(parent, "img")
        img.set('src', '/' + svg_file)
        img.set('style', 'background:transparent; border:0px')
