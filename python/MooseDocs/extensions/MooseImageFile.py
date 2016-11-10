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
    el = etree.Element('div')
    self.addStyle(el, **styles)

    card = etree.SubElement(el, 'div')
    card.set('class', 'card')

    img_card = etree.SubElement(card, 'div')
    img_card.set('class', 'card-image')

    img = etree.SubElement(img_card, 'img')
    img.set('src', rel_filename)
    img.set('class', 'materialboxed')

    # Add caption
    if settings['caption']:
      caption = etree.SubElement(card, 'div')
      p = etree.SubElement(caption, 'p')
      p.set('class', 'moose-caption')
      p.set('align', "justify")
      p.text = settings['caption']

    return el
