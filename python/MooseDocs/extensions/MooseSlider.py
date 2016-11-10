from markdown.blockprocessors import BlockProcessor
from MooseCommonExtension import MooseCommonExtension
import glob
import re
import os
from markdown.util import etree

class MooseSlider(BlockProcessor, MooseCommonExtension):
  """
  Markdown extension for showing a Materialize carousel of images.
  Markdown syntax is:

   !slider <options>
     images/intro.png caption=Some caption
     images/more*.png

  Where <options> are key=value pairs.
  See http://getbootstrap.com/javascript/#carousel for allowed options.
  Additionally, "caption" can also be used on the slideshow line to
  set a default caption.

  It is assumed image names will have the same filepath as on the webserver.
  """

  RE = re.compile(r'^!\ ?slideshow(.*)')
  # If there are multiple carousels on the same page then
  # they need to have different ids
  MATCHES_FOUND = 0

  def __init__(self, parser, **kwargs):
    MooseCommonExtension.__init__(self, **kwargs)
    BlockProcessor.__init__(self, parser)

    # The default settings
    self._settings = {'caption'  : None,
                      'interval' : None,
                      'pause'    : None,
                      'wrap'     : None,
                      'keyboard' : None}

  def parseFilenames(self, filenames_block):
    """
    Parse a set of lines with filenames in them and an optional caption.
    Filenames can contain wildcards and glob will be used to expand them.
    Expected input is similar to:
      images/1.png caption=My caption
      images/other*.png
    Input:
     filenames_block[str]: String block to parse
    Return:
     list of dicts. Each dict has keys of "path" which is the filename path
      and "caption" which is the associated caption. Caption will be "" if not
      specified.
    """
    lines = filenames_block.split("\n")
    files = []
    for line in lines:
      sline = line.strip()
      idx = sline.find("caption=")
      if idx >=0 :
        caption = sline[idx+8:].strip()
        fname = sline[:idx].strip()
      else:
        caption = ""
        fname = sline

      new_files = glob.glob(os.path.join(self._docs_dir, fname))
      if not new_files:
        # If one of the paths is broken then
        # we return an empty list to indicate
        # an error state
        return []
      for f in new_files:
        files.append({"path": os.path.relpath(f, self._docs_dir), "caption": caption})
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

    ul = etree.SubElement(slider, 'ul')
    ul.set('class', 'slides')

    for item in self.parseFilenames(block[match.end()+1:]):
      li = etree.SubElement(ul, 'li')
      img = etree.SubElement(li, 'img')
      img.set('src', '/' + item['path'])

      if item['caption']:
        caption = etree.SubElement(li, 'div')
        caption.text = item['caption']
