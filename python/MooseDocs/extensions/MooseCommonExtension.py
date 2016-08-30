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
      for entry in matches:
        options[entry[0].strip()] = entry[1].strip()
      return options

    def createErrorElement(self, rel_filename='', message=None):
        """
        Returns a tree element containing error message.

        Uses the html to match the python markdown admonition package.
        https://pythonhosted.org/Markdown/extensions/admonition.html

        <div class="admonition danger">
        <p class="admonition-title">Don't try this at home</p>
        <p>...</p>
        </div>
        """

        el = etree.Element('div')
        el.set('class', "admonition danger")

        title = etree.SubElement(el, 'p')
        title.set('class', "admonition-title")
        title.text = "Markdown Parsing Error"

        msg = etree.SubElement(el, 'p')
        msg.text = 'Invalid markdown for ' + rel_filename
        if message:
            msg.text += '<br>{}'.format(message)
        log.error(msg.text)
        return el
