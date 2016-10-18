import re
import os
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension
import utils

class MooseImageFile(MooseCommonExtension, Pattern):
  """
  Markdown extension for handling images.

  Usage:
   !image image_file.png|jpg|etc attribute=setting

  Settings:
    caption[str]: Creates a figcaption tag with the supplied text applied.

  All image filenames should be supplied as relative to the docs directory, i.e., media/my_image.png
  """

  # Find !image /path/to/file attribute=setting
  RE = r'^!image\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, markdown_instance=None, **kwargs):
    MooseCommonExtension.__init__(self, **kwargs)
    Pattern.__init__(self, self.RE, markdown_instance)

    # Valid settings for MOOSE specific documentation features
    # All other markdown 'attributes' will be treated as HTML
    # style settings for the figure tag.
    self._settings = {'caption' : None}

  def handleMatch(self, match):
    """
    process settings associated with !image markdown
    """

    # A tuple separating specific MOOSE documentation features (self._settings) from HTML styles
    settings, styles = self.getSettings(match.group(3))

    # Read the file and create element
    filename = os.path.join(self._root, self._docs_dir, match.group(2))
    rel_filename = '/' + os.path.relpath(filename, os.path.join(self._root, self._docs_dir))
    if not os.path.exists(filename):
      return self.createErrorElement('File not found: {}'.format(rel_filename))

    # Create the figure element
    fig = etree.Element('figure')
    self.addStyle(fig, **styles)

    # Add the image
    img = etree.SubElement(fig, 'img')
    img.set('src', rel_filename)

    # Add caption
    if settings['caption']:
      caption = etree.SubElement(fig, 'figcaption')
      p = etree.SubElement(caption, 'p')
      p.set('align', "justify")
      p.text = settings['caption']

    return fig
