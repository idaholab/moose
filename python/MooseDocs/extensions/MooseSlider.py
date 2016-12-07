from markdown.blockprocessors import BlockProcessor
from MooseCommonExtension import MooseCommonExtension
import glob
import re
import os
import string
from markdown.util import etree

import inspect

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

      #get separate dictionaries for the image and caption
      img_dict = dict()
      caption_dict = dict()
      dict_array = [img_dict,caption_dict]
      for i in [2,3]:
        settings, styles = self.getSettings(matches.group(i).strip())
        dict_array[i-2].update(settings)
        dict_array[i-2].update(styles)

      new_files = glob.glob(os.path.join(self._docs_dir, fname))
      if not new_files:
        # If one of the paths is broken then
        # we return an empty list to indicate
        # an error state
        print '\n\nWARNING!  Parser unable to detect file(s) "%s" in MooseSlider.py\n'%(fname)
        return []
      for f in new_files:
        files.append(({"path": os.path.relpath(f, self._docs_dir)},img_dict,caption_dict))
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
    options = match.group(1)
    settings, styles = self.getSettings(options)

    slider = etree.SubElement(parent, 'div')
    slider.set('class', 'slider')
    styles.update(settings)
    slider = self.addStyle(slider, **styles)

    ul = etree.SubElement(slider, 'ul')
    ul.set('class', 'slides')

    for item in self.parseFilenames(block[match.end()+1:]):
      li = etree.SubElement(ul, 'li')
      img = etree.SubElement(li, 'img')
      img_dict = {'background-size':'contain', 'background-repeat':'no-repeat', 'background-color':'white'}
      img_dict.update(item[1])
      img.set('src', '/' + item[0]['path'])
      img = self.addStyle(img, **img_dict)

      #Add the caption and its options if they exist
      if len(item[2]) != 0:
        caption = etree.SubElement(li, 'div')
        caption.set('class','caption')
        caption.text = item[2]['caption']
        styles = item[2]
        del styles['caption']
        caption = self.addStyle(caption, **styles)
