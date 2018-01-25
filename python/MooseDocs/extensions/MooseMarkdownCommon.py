#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
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
    SETTINGS_RE = r'([^\s=]+)=(.*?)(?=(?:\s[^\s=]+=|$))'
    COPY_BUTTON_COUNT = 0
    HELP_CONTENT_COUNT = 0

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

    def getSettings(self, settings_line, legacy_style=True):
        """
        Parses a string of space separated key=value pairs. This supports having values with spaces
        in them. So something like "key0=foo bar key1=value1" is supported.

        Input:
            settings_line[str]: Line to parse
            legacy_style[bool]: When True all unknown parameters are but into the "style", this
                                assumption needs to go away and this parameter will allow it to
                                be deprecated.
        Returns:
            dict of values that were parsed
        """
        return self.getSettingsHelper(copy.copy(self.__settings), settings_line, legacy_style)

    @staticmethod
    def getSettingsHelper(options, settings_line, legacy_style=True):
        """
        see getSettings
        """
        # Crazy RE capable of many things like understanding key=value pairs with spaces in them!
        matches = re.findall(MooseMarkdownCommon.SETTINGS_RE, settings_line.strip())

        if len(matches) == 0:
            return options
        for entry in matches:
            key = entry[0].strip()
            value = entry[1].strip()
            if value.lower() == 'true':
                value = True
            elif value.lower() == 'false':
                value = False
            elif value.lower() == 'none':
                value = None
            elif all([v.isdigit() for v in value]):
                value = float(value) #pylint: disable=redefined-variable-type

            if legacy_style and (key in options.keys()):
                options[key] = value
            elif legacy_style:
                options['style'] += '{}:{};'.format(key, value)
            else:
                options[key] = value

        return options

    def getFilename(self, *args, **kwargs):
        """
        Locate nodes with a filename ending with provided string.
        """
        return self.markdown.getFilename(*args, **kwargs) #pylint: disable=no-member

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

        if settings.get('id', False) or settings.get('caption', False):
            p = etree.SubElement(div, 'p')
            p.set('class', 'moose-float-caption')

            if settings.get('id', None) is not None:
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

            if settings.get('caption', None) is not None:
                t_span = etree.SubElement(p, 'span')
                t_span.set('class', 'moose-float-caption-text')
                t_span.text = settings['caption']

        return div

    def createErrorElement(self, message, title='Markdown Parsing Error', parent=None, error=True,
                           help_button=None, markdown=False):
        """
        Returns a tree element containing error message.

        Uses the html to match the python markdown admonition package.
        https://pythonhosted.org/Markdown/extensions/admonition.html

        <div class="admonition error">
        <p class="admonition-title">Don't try this at home</p>
        <p>...</p>
        </div>

        Args:
            message[str]: The message to display in the alert box
            title[str]: Set the title (default: "Markdown Parsing Error")
            parent[etree.Element]: The parent element that should contain the error message
            error[bool]: LOG an error (default: True)
            help_button[etree.Element]: The html button to use for creating a help pop-up
            markdown[bool]: When true this will enable markdown parsing within html
        """
        el = self.createAdmonition('error', message, title=title, parent=parent,
                                   help_button=help_button, markdown=markdown)
        if error:
            LOG.error('%s: %s', str(title), str(message))
        return el

    def createAdmonition(self, command, message, title=None, parent=None, help_button=None,
                         markdown=False):
        """
        Generates Element object for MOOSE admonitions
        """

        if parent is not None:
            el = etree.SubElement(parent, 'div')
        else:
            el = etree.Element('div')
        el.set('class', "admonition {}".format(command))

        self.markdown.current.status[command] += 1 #pylint: disable=no-member

        title_div = etree.SubElement(el, 'div')
        title_el = etree.SubElement(title_div, 'p')
        title_el.set('class', "admonition-title")
        if title:
            title_el.text = '{}: {}'.format(command.title(), title)
        else:
            title_el.text = command.title()

        msg_div = etree.SubElement(el, 'div')
        msg_div.set('class', 'admonition-message')

        msg = etree.SubElement(msg_div, 'p')
        msg.text = message
        if markdown:
            msg.set('markdown', '1')

        # Add the help button
        if help_button is not None:
            id_ = 'moose-modal-help-data-{}'.format(MooseMarkdownCommon.HELP_CONTENT_COUNT)

            a = etree.SubElement(title_el, 'a')
            a.set('class', 'waves-effect waves-light btn-floating red')
            a.set('data-target', id_)
            a.set('style', 'float:right')
            i = etree.SubElement(a, 'i')
            i.set('class', 'material-icons')
            i.text = 'help'

            modal = etree.SubElement(el, 'div')
            modal.set('id', id_)
            modal.set('class', 'modal')

            content = etree.SubElement(modal, 'div')
            content.set('class', 'modal-content')
            content.append(help_button)

            MooseMarkdownCommon.HELP_CONTENT_COUNT += 1

        return el
