import os
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension

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
        self._settings['caption'] = None
        self._settings['center'] = False

    def createImageElement(self, rel_filename, settings):
        """
        Create the element containing the image, this is a separate function to allow for other objects
        (i.e., MooseFigure) to utilize this class to build similar html.

        Inputs:
          rel_filename[str]: The path to the image relative to the git repository.
          settings[dict]: The settings extracted via getSettings() method.
        """
        # Read the file and create element
        filename = None
        repo = MooseDocs.abspath(rel_filename)
        local = os.path.abspath(os.path.join(os.getcwd(), rel_filename))
        if os.path.exists(repo):
            filename = repo
        elif os.path.exists(local):
            filename = local
        else:
            return self.createErrorElement('File not found: {}'.format(rel_filename))

        # Create the figure element
        card = self.applyElementSettings(etree.Element('div'), settings)
        card.set('class', 'card')

        if settings['center']:
            style = card.get('style', '')
            card.set('style', 'margin-left:auto;margin-right:auto;' + style)

        img_card = etree.SubElement(card, 'div')
        img_card.set('class', 'card-image')

        img = etree.SubElement(img_card, 'img')
        img.set('src', os.path.relpath(filename, os.getcwd()))
        img.set('class', 'materialboxed')
        img.set('style', 'margin-left:auto;margin-right:auto;margin-top:auto;margin-bottom:auto')

        # Add caption
        if settings['caption']:
            caption = etree.SubElement(card, 'div')
            p = etree.SubElement(caption, 'p')
            p.set('class', 'moose-caption')
            p.set('align', "justify")
            p.text = settings['caption']

        return card

    def handleMatch(self, match):
        """
        process settings associated with !image markdown
        """
        rel_filename = match.group(2)
        settings = self.getSettings(match.group(3))
        return self.createImageElement(rel_filename, settings)
