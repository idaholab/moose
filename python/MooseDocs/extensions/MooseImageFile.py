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
    RE = r'^!\ ?image\s+(.*?)\s+(.*)'

    def __init__(self, media_dir=None, root=None, **kwargs):
        MooseCommonExtension.__init__(self)
        Pattern.__init__(self, self.RE, **kwargs)

        self._root = os.path.join(root, media_dir)

        # The default settings
        self._settings = {'alt'     : None,
                          'caption' : None,
                          'width'   : None,
                          'height'  : None,
                          'float'   : None,
                          'align'   : 'left'}



    def handleMatch(self, match):
        """
        process settings associated with !image markdown
        """

        rel_filename = match.group(2)

        # Perform second regex to capture key=value pairs with
        # possible spaces in the value
        settings = self.getSettings(match.group(3))

        # Read the file and create element
        filename = self.checkFilename(rel_filename)

        if not filename:
            print '{}'.format(rel_filename)
            el = self.createErrorElement(rel_filename, message='file not found')
        else:
            # Use the more friendly term 'caption' within markdown, but make
            # the HTML tag attribute 'alt' correction
            if settings['caption'] != None:
                settings['alt'] = settings['caption']

            # When aligning to one side or another, we need to adjust the margins
            # on the opisite side... silly looking buy necessary
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
            for attribute in settings.keys():
                if attribute == 'align':
                    el_list['div'].set('style', 'text-align: -moz-{}; text-align: -webkit-{};'.format(settings[attribute], settings[attribute]))
                elif attribute == 'float' and settings[attribute] is not None:
                    el_list['figure'].set('style', el_list['figure'].get('style') + \
                                          'float: {}; margin-{}: 20px'.format(settings[attribute], reverse_margin[settings[attribute]]))
                elif settings[attribute] != None:
                    el_list['img'].set(attribute, str(settings[attribute]))

            # if caption/alt set, add figcaption
            if settings['alt'] != None:
                # Unset the large default bottom-margin for figcaption
                el_list['img'].set('style', 'margin-bottom: unset;')
                el_list['figcaption'] = etree.SubElement(el_list['figure'], 'figcaption')
                el_list['figcaption'].text = settings['alt']

            el = el_list['div']

        return el
