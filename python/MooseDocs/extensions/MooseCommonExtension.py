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
  def __init__(self, **kwargs):

    # The default settings should be stored here
    self._settings = {'id': None, 'class': None, 'style': ''}

  def getSettings(self, settings_line):
    """
    Parses a string of space separated key=value pairs.
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

    options = copy.copy(self._settings)
    if len(matches) == 0:
      return options
    for entry in matches:
     if entry[0] in options.keys():
       options[entry[0].strip()] = entry[1].strip()
     else:
      # log.warning('The style should be specified in the style argument (e.g., style="{}:{};")'.format(entry[0].strip(), entry[1].strip()))
       options['style'] += '{}:{};'.format(entry[0].strip(), entry[1].strip())
    return options


  @staticmethod
  def applyElementSettings(element, settings):
    """
    Returns supplied element with style attributes.

    Useful for adding things like; sizing, floating,
    padding, margins, etc to any element.

    Usage:
      settings = self.getSettings(settings_string)
      div = self.appyElementSettings(etree.Element('div'), settings)
    """
    for attr in ['id', 'class', 'style']:
      if settings[attr] != None:
        element.set(attr, settings[attr])
    return element


  def createErrorElement(self, message, title='Markdown Parsing Error', parent=None, warning=False):
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
        warning[bool]: Create a warning rather than an error (default: False)
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
    if warning:
      log.warning('{}: {}'.format(title, message))
    else:
      log.error('{}: {}'.format(title, message))
    return el
