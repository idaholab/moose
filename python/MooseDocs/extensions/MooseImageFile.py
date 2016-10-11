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

    All images/media should be stored in docs/media
    """

    # Find !image /path/to/file attribute=setting
    RE = r'^!image\s+(.*?)(?:$|\s+)(.*)'

    def __init__(self, markdown_instance=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

        # Valid settings for MOOS specific documentation features
        # All other markdown 'attributes' will be treated as HTML
        # style settings
        self._settings = {'caption' : None}

    def handleMatch(self, match):
        """
        process settings associated with !image markdown
        """

        rel_filename = match.group(2)

        # A tuple separating specific MOOSE documentation features (self._settings) from HTML styles
        settings, styles = self.getSettings(match.group(3))

        # Read the file and create element
        filename = os.path.join(self._root, self._docs_dir, rel_filename)
        if not os.path.exists(filename):
            el = self.createErrorElement('File not found: {}'.format(rel_filename))
        else:
            # When aligning to one side or another, we need to adjust the margins
            # on the opposite side... silly looking buy necessary
            reverse_margin = { 'left' : 'right',
                               'right' : 'left',
                               'None' : 'none'}

            el_list = {}
            el_list['div'] = etree.Element('div')
            el_list['figure'] = etree.SubElement(el_list['div'], 'figure')
            el_list['img'] = etree.SubElement(el_list['figure'], 'img')
            el_list['img'].set('src', os.path.join('/media', os.path.basename(filename)))

            # Set the default figcaption text alignment
            el_list['figure'].set('style', 'text-align: left; display: table;')

            # Set any extra supplied attributes
            if el_list['div'].get('style') != None:
                previous_style = el_list['div'].get('style')
            else:
                previous_style = ''
            for attribute in styles.keys():
                if attribute == 'align':
                    el_list['div'].set('style', ';'.join(['text-align: -moz-{}; text-align: -webkit-{};'.format(styles[attribute],
                                                                                                                styles[attribute]),
                                                          previous_style]))
                elif attribute == 'float' and styles[attribute] is not None:
                    el_list['figure'].set('style', el_list['figure'].get('style') + \
                                          'float: {}; margin-{}: 20px'.format(styles[attribute], reverse_margin[styles[attribute]]))
                elif styles[attribute] != None:
                    el_list['img'].set(attribute, str(styles[attribute]))

            # if caption set, add figcaption
            if settings['caption'] != None:
                # Unset the large default bottom-margin for figcaption
                el_list['img'].set('style', 'margin-bottom: unset;')
                el_list['figcaption'] = etree.SubElement(el_list['figure'], 'figcaption')
                el_list['figcaption'].text = settings['caption']

            el = el_list['div']

        return el
