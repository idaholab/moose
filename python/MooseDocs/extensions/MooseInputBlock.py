import os
import MooseDocs
from MooseTextPatternBase import MooseTextPatternBase
from FactorySystem import ParseGetPot

class MooseInputBlock(MooseTextPatternBase):
  """
  Markdown extension for extracting blocks from input files.
  """

  CPP_RE = r'^!input\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, **kwargs):
    MooseTextPatternBase.__init__(self, self.CPP_RE, language='text', **kwargs)
    self._settings['block'] = None

  def handleMatch(self, match):
    """
    Process the input file supplied.
    """

    # Update the settings from regex match
    settings = self.getSettings(match.group(3))

    # Build the complete filename.
    rel_filename = match.group(2)
    filename = MooseDocs.abspath(rel_filename)

    # Read the file and create element
    if not os.path.exists(filename):
      el = self.createErrorElement("The input file was not located: {}".format(rel_filename))
    elif not settings.has_key('block'):
      el = self.createErrorElement("Use of !input syntax while not providing a block=some_block. If you wish to include the entire file, use !text instead")
    else:
      parser = ParseGetPot(filename)
      node = parser.root_node.getNode(settings['block'])

      if node == None:
        content = 'ERROR: Failed to find {} in {}.'.format(match.group(2), rel_filename)
      else:
        content = node.createString()

      if match.group(2):
        label = match.group(2)
      else:
        label = rel_filename
      el = self.createElement(label, content, filename, rel_filename, settings)

    return el
