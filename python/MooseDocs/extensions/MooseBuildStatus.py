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

    def __init__(self, **kwargs):
        MooseCommonExtension.__init__(self)
        Pattern.__init__(self, self.RE, **kwargs)

        # We have a strict width set that can not be disturbed
        self._invalid_css = { 'div' : ['width'] }

    def handleMatch(self, match):
        """
        process settings associated with !buildstatus markdown
        """
        reverse_margin = { 'left' : 'right',
                           'right' : 'left',
                           'None' : 'none'}

        url = match.group(2)

        # A tuple separating specific MOOSE documentation features (self._settings) from HTML styles
        settings, styles = self.getSettings(match.group(3))

        # Create parent div, and set any allowed CSS
        parent_div = self.addStyle(etree.Element('div'), **styles)

        child_div = etree.SubElement(parent_div, 'div')
        jquery_script = etree.SubElement(parent_div, 'script')
        build_status_script = etree.SubElement(parent_div, 'script')

        jquery_script.set('src', 'http://code.jquery.com/jquery-1.11.0.min.js')

        # We need to inform SmartyPants to not format for paragraph use
        build_status_script.text = AtomicString('$(document).ready(function(){ $("#buildstatus").load("%s");});' % (url))

        # Set some necessary defaults for our child div
        child_div.set('id', 'buildstatus')

        # work with the special case of align or float
        if 'align' in styles.keys() or 'float' in styles.keys():
            if 'align' in styles.keys():
                position = styles['align']
                r_position = reverse_margin[styles['align']]

                parent_css_style = ';'.join(['width: 100%; text-align: -moz-{}; text-align: -webkit-{};'.format(position, position), parent_div.get('style')])
                child_css_style = 'text-align: -moz-{}; text-align: -webkit-{};'.format(r_position, r_position)

                # drop any further use of 'align'
                styles.pop('align', None)
            else:
                position = styles['float']
                r_position = reverse_margin[styles['float']]

                parent_css_style = ';'.join(['width: 25%; text-align: -moz-{}; text-align: -webkit-{};'.format(position, position), parent_div.get('style')])
                child_css_style = 'text-align: -moz-{}; text-align: -webkit-{};'.format(r_position, r_position)
                # drop any further use of 'float'
                styles.pop('float', None)

            parent_div.set('style', parent_css_style)
            child_div.set('style', child_css_style)

        # Set any additional allowed CSS
        child_div = self.addStyle(child_div, **styles)
        return parent_div
