import re
import os
import copy
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
import utils

class MooseCommonExtension(object):
    """
    Class containing commonly used routines.

    """
    def __init__(self):
        self._settings = dict() # The default settings should be stored here

    def checkFilename(self, rel_filename):
        """
        Checks that the filename exists, if it does not a error Element is return.

        Args:
            filename[str]: The filename to check for existence.
        """
        filename = os.path.abspath(os.path.join(self._root, rel_filename))
        if os.path.exists(filename):
            return filename
        return None

    def getSettings(self, settings_line):
      """
      Parses a string of space seperated key=value pairs.
      This supports having values with spaces in them.
      So something like "key0=foo bar key1=value1"
      is supported.
      Input:
        settings_line[str]: Line to parse
      Returns:
        dict of values that were parsed
      """

      # Crazy RE capable of many things
      # like understanding key=value pairs with spaces in them!
      SETTINGS_RE = re.compile("([^\s=]+)=(.*?)(?=(?:\s[^\s=]+=|$))")
      matches = SETTINGS_RE.findall(settings_line.strip())

      if not matches:
        return {}
      options = copy.copy(self._settings)
      styles = {}
      for entry in matches:
        if entry[0] in options.keys():
            options[entry[0].strip()] = entry[1].strip()
        else:
            styles[entry[0].strip()] = entry[1].strip()
      return options, styles

    def addStyle(self, element, **kwargs):
        """
        Returns supplied element with style attributes.

        Useful for adding things like; sizing, floating,
        padding, margins, etc to any element.

        Usage:
        addStyle(etree.Element, width='300px')

        returns your element with style="with=300px;" along
        with any pre-existing styles set.

        """
        if kwargs == {}:
            return element
        else:
            for attribute, value in kwargs.iteritems():
                if self._invalid_css.has_key(element.tag) and (attribute in self._invalid_css[element.tag]):
                    continue
                if element.get('style') is not None:
                    element.set('style', ';'.join([':'.join([attribute,value]), element.get('style')]))
                else:
                    element.set('style', ':'.join([attribute,value]) + ';')
            return element

    def createErrorElement(self, rel_filename='', message=None, title='Markdown Parsing Error', parent=None):
        """
        Returns a tree element containing error message.

        Uses the html to match the python markdown admonition package.
        https://pythonhosted.org/Markdown/extensions/admonition.html

        <div class="admonition error">
        <p class="admonition-title">Don't try this at home</p>
        <p>...</p>
        </div>
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
        msg.text = 'Invalid markdown for ' + rel_filename
        if message:
            msg.text = message
        log.error('{}\n{}'.format(title, message))
        return el
