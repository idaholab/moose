import re
import os
from markdown.util import etree
from markdown.util import AtomicString
import logging
log = logging.getLogger(__name__)

import MooseDocs
from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension
import utils

class MooseBuildStatus(MooseCommonExtension, Pattern):
  """
  Markdown extension for add Build Status widget.

  Usage:
   !buildstatus http://civet/buildstatus/site css_attribute=setting

  """

  # Find !buildstatus url attribute=value
  RE = r'^!buildstatus\s+(.*?)(?:$|\s+)(.*)'

  def __init__(self, markdown_instance=None, **kwargs):
    MooseCommonExtension.__init__(self, **kwargs)
    Pattern.__init__(self, self.RE, markdown_instance)

  def handleMatch(self, match):
    """
    process settings associated with !buildstatus markdown
    """
    reverse_margin = { 'left' : 'right',
                       'right' : 'left',
                       'None' : 'none'}

    url = match.group(2)

    # A tuple separating specific MOOSE documentation features (self._settings) from HTML styles
    settings = self.getSettings(match.group(3))

    # Create parent div, and set any allowed CSS
    parent_div = self.applyElementSettings(etree.Element('div'), settings)
    parent_div.set('class', 'moose-buildstatus')

    child_div = etree.SubElement(parent_div, 'div')
    jquery_script = etree.SubElement(parent_div, 'script')
    build_status_script = etree.SubElement(parent_div, 'script')

    jquery_script.set('src', 'http://code.jquery.com/jquery-1.11.0.min.js')

    # We need to inform SmartyPants to not format for paragraph use
    build_status_script.text = AtomicString('$(document).ready(function(){ $("#buildstatus").load("%s");});' % (url))

    # Set some necessary defaults for our child div
    child_div.set('id', 'buildstatus')

    # Set any additional allowed CSS
    return parent_div
