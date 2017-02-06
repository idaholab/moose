import glob
import re
import os
from markdown.util import etree
from markdown.blockprocessors import BlockProcessor
import collections
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseCommonExtension import MooseCommonExtension


class MooseSlider(BlockProcessor, MooseCommonExtension):
    """
    Markdown extension for showing a Materialize carousel of images.
    Markdown syntax is:

     !slider <options>
       images/intro.png <image_options> caption=Some caption <caption_options>
       images/more*.png

    Where <options> are key=value pairs.
    Valid options are standard CSS options (images are set as background images).

    It is assumed image names will have the same filepath as on the webserver.
    """

    RE = re.compile(r'^!\ ?slider(.*)')

    ImageInfo = collections.namedtuple('ImageInfo', 'filename img_settings caption_settings')

    def __init__(self, parser, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        BlockProcessor.__init__(self, parser)

    def parseFilenames(self, filenames_block):
        """
        Parse a set of lines with filenames, image options, and optional captions.
        Filenames can contain wildcards and glob will be used to expand them.
        Any CSS styles after the filename (but before caption if it exists)
        will be applied to the image (image is set as a background in slider).
        CSS styles listed after the caption will be applied to it.
        Expected input is similar to:
          images/1.png caption=My caption color=blue
          images/2.png background-color=gray caption= Another caption color=red
          images/other*.png
        Input:
         filenames_block[str]: String block to parse
        Return:
         list of list of dicts. The list has an entry for each image (including
         one for each expanded image from glob), each entry contains:
         1. dict of "path" which is the filename path
         2. dict of attributes to be applied to the image
         3. dict of attributes to be applied to the caption
         Each image will default to fit the slideshow window with white background
         and no caption if no options are specified.
        """
        lines = filenames_block.split("\n")
        files = []
        regular_expression = re.compile(r'(.*?\s|.*?$)(.*?)(caption.*|$)')
        for line in lines:
            line = line.strip()
            matches = regular_expression.search(line)
            fname = matches.group(1).strip()

            # Build separate dictionaries for the image and caption
            img_settings = self.getSettings(matches.group(2).strip())
            img_settings.setdefault('background-size', 'contain')
            img_settings.setdefault('background-repeat', 'no-repeat')
            img_settings.setdefault('background-color', 'white')
            caption_settings = self.getSettings(matches.group(3).strip())

            new_files = glob.glob(MooseDocs.abspath(fname))
            if not new_files:
                log.error('Parser unable to detect file(s) {} in MooseSlider.py'.format(fname))
                return []
            for f in new_files:
                files.append(MooseSlider.ImageInfo(os.path.relpath(f), img_settings, caption_settings))

        return files

    def test(self, parent, block):
        """
        Test to see if we should process this block of markdown.
        Inherited from BlockProcessor.
        """
        return self.RE.search(block)

    def run(self, parent, blocks):
        """
        Called when it is determined that we can process this block.
        This will convert the markdown into HTML
        """

        block = blocks.pop(0)
        match = self.RE.search(block)
        settings = self.getSettings(match.group(1))

        slider = etree.SubElement(parent, 'div')
        slider.set('class', 'slider')
        self.applyElementSettings(slider, settings)

        ul = etree.SubElement(slider, 'ul')
        ul.set('class', 'slides')

        for item in self.parseFilenames(block[match.end()+1:]):
            li = etree.SubElement(ul, 'li')
            img = etree.SubElement(li, 'img')
            img.set('src', item.filename)
            self.applyElementSettings(img, item.img_settings, keys=item.img_settings.keys())

            #Add the caption and its options if they exist
            if len(item[2]) != 0:
                caption = etree.SubElement(li, 'div')
                caption.set('class','caption')
                caption.text = item.caption_settings.pop('caption', '')
                caption = self.applyElementSettings(caption, item.caption_settings, keys=item.caption_settings.keys())
