#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import re
import copy
import logging

from markdown.util import etree

LOG = logging.getLogger(__name__)

class MooseMarkdownCommon(object):
    """
    Class containing commonly used routines, which should always be used with a markdown instance
    such as a Pattern or other object that has a "self.markdown" attribute.
    """
    COPY_BUTTON_COUNT = 0

    @staticmethod
    def defaultSettings():
        """Default settings for MooseMarkdownCommon."""
        settings = dict()
        settings['id'] = (None, "The HTML element 'id' for the outer tag.")
        settings['style'] = ('', "The 'style' to be applied to the outer HTML tag.")
        return settings

    def __init__(self, **kwargs): #pylint: disable=unused-argument

        # Store the default settings
        self.__settings = dict()
        for key, value in self.defaultSettings().iteritems():
            self.__settings[key] = value[0]

    def getSettings(self, settings_line):
        """
        Parses a string of space separated key=value pairs. This supports having values with spaces
        in them. So something like "key0=foo bar key1=value1" is supported.

        Input:
          settings_line[str]: Line to parse
        Returns:
          dict of values that were parsed
        """

        # Crazy RE capable of many things like understanding key=value pairs with spaces in them!
        matches = re.findall(r'([^\s=]+)=(.*?)(?=(?:\s[^\s=]+=|$))', settings_line.strip())

        options = copy.copy(self.__settings)
        if len(matches) == 0:
            return options
        for entry in matches:
            key = entry[0].strip()
            value = entry[1].strip()
            if key in options.keys():
                if value.lower() == 'true':
                    value = True
                elif value.lower() == 'false':
                    value = False
                elif value.lower() == 'none':
                    value = None
                elif all([v.isdigit() for v in value]):
                    value = float(value) #pylint: disable=redefined-variable-type
                options[key] = value
            else:
                options['style'] += '{}:{};'.format(key, value)

        return options


    @staticmethod
    def applyElementSettings(element, settings, keys=None):
        """
        Returns supplied element with style attributes.

        Useful for adding things like; sizing, floating,
        padding, margins, etc to any element.

        Usage:
          settings = self.getSettings(settings_string)
          div = self.appyElementSettings(etree.Element('div'), settings)
        """
        for attr in keys if keys else ['id', 'class', 'style']:
            if (attr in settings) and (settings[attr]):
                element.set(attr, settings[attr])
        return element

    def createFloatElement(self, settings):
        """
        Returns an html <div> element suitable for figures, tables, etc.
        """

        cname = settings['counter']
        if cname is None:
            LOG.error('The "counter" setting must be a valid string (%s)',
                      self.markdown.current.source()) #pylint: disable=no-member
            cname = 'unknown'

        div = self.applyElementSettings(etree.Element('div'), settings)
        div.set('class', 'moose-float-div moose-{}-div'.format(cname))

        if settings.get('id', None) or settings.get('caption', None):
            p = etree.SubElement(div, 'p')
            p.set('class', 'moose-float-caption')

            if settings.get('id', None):
                div.set('data-moose-float-name', cname.title())
                h_span = etree.SubElement(p, 'span')
                h_span.set('class', 'moose-float-caption-heading')

                h_span_text = etree.SubElement(h_span, 'span')
                h_span_text.set('class', 'moose-float-caption-heading-label')
                h_span_text.text = cname.title() + ' '

                h_span_num = etree.SubElement(h_span, 'span')
                h_span_num.set('class', 'moose-float-caption-heading-number')
                h_span_num.text = '??'

                h_span_suffix = etree.SubElement(h_span, 'span')
                h_span_suffix.set('class', 'moose-float-caption-heading-suffix')
                h_span_suffix.text = ': '

            if settings.get('caption', None):
                t_span = etree.SubElement(p, 'span')
                t_span.set('class', 'moose-float-caption-text')
                t_span.text = settings['caption']

        return div

    @staticmethod
    def createErrorElement(message, title='Markdown Parsing Error', parent=None, error=True):
        """
        Returns a tree element containing error message.

        Uses the html to match the python markdown admonition package.
        https://pythonhosted.org/Markdown/extensions/admonition.html

        <div class="admonition error">
        <p class="admonition-title">Don't try this at home</p>
        <p>...</p>
        </div>

        Args:
            message[str]: The message to display in the alert box.
            title[str]: Set the title (default: "Markdown Parsing Error")
            parent[etree.Element]: The parent element that should contain the error message
            error[bool]: LOG an error (default: True).
        """
        if parent:
            el = etree.SubElement(parent, 'div')
        else:
            el = etree.Element('div')
        el.set('class', "admonition error")

        title_el = etree.SubElement(el, 'p')
        title_el.set('class', "admonition-title")
        title_el.text = title

        msg = etree.SubElement(el, 'p')
        msg.text = message
        if error:
            LOG.error('%s: %s', title, message)
        return el

    @staticmethod
    def createCopyButton(tag_id):
        """
        Create a copy button.
        """
        btn = etree.Element('button')
        btn.set('class', 'moose-copy-button btn')
        btn.set('data-clipboard-target', '#{}'.format(tag_id))
        btn.text = 'copy'
        return btn
