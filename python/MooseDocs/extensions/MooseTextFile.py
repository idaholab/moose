import re
import os
import MooseDocs
from MooseTextPatternBase import MooseTextPatternBase

class MooseTextFile(MooseTextPatternBase):
  """
  A markdown extension for including complete source code files.
  """
  RE = r'^!text\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, **kwargs):
    super(MooseTextFile, self).__init__(self.RE, **kwargs)

    # Add to the default settings
    self._settings['line'] =  None
    self._settings['start'] =  None
    self._settings['end'] =  None
    self._settings['include_end'] = False

  def handleMatch(self, match):
    """
    Process the text file provided.
    """

    # Update the settings from regex match
    settings, styles = self.getSettings(match.group(3))

    # Read the file
    rel_filename = match.group(2).lstrip('/')
    filename = MooseDocs.abspath(rel_filename)
    if not os.path.exists(filename):
      return self.createErrorElement("Unable to locate file: {}".format(rel_filename))
    if settings['line']:
      content = self.extractLine(filename, settings["line"])

    elif settings['start'] or settings['end']:
      content = self.extractLineRange(filename, settings['start'], settings['end'], settings['include_end'])

    else:
      with open(filename) as fid:
        content = fid.read()

    if content == None:
      return self.createErrorElement("Failed to extract content from {}.".format(filename))

    # Return the Element object
    el = self.createElement(match.group(2), content, filename, rel_filename, settings, styles)
    return el

  def extractLine(self, filename, desired):
    """
    Function for returning a single line.

    Args:
      desired[str]: The text to look for within the source file.
    """

    # Read the lines
    with open(filename) as fid:
      lines = fid.readlines()

    # Search the lines
    content = None
    for line in lines:
      if desired in line:
        content = line

    return content

  def extractLineRange(self, filename, start, end, include_end):
    """
    Function for extracting content between start/end strings.

    Args:
      filename[str]: The name of the file to examine.
      start[str|None]: The starting line (when None is provided the beginning is used).
      end[str|None]: The ending line (when None is provided the end is used).
      include_end[bool]: If True then the end string is included
    """

    # Read the lines
    with open(filename) as fid:
      lines = fid.readlines()

    # The default start/end positions
    start_idx = 0
    end_idx = len(lines)

    if start:
      for i in range(end_idx):
        if start in lines[i]:
          start_idx = i
          break
    if end:
      for i in range(start_idx, end_idx):
        if end in lines[i]:
          end_idx = i + 1 if include_end else i
          break

    return ''.join(lines[start_idx:end_idx])
