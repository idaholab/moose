import re
import copy
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

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
                    value = float(value)
                options[key] = value
            else:
                options['style'] += '{}:{};'.format(key, value)

        return options


    @staticmethod
    def applyElementSettings(element, settings, keys=['id', 'class', 'style']):
        """
        Returns supplied element with style attributes.

        Useful for adding things like; sizing, floating,
        padding, margins, etc to any element.

        Usage:
          settings = self.getSettings(settings_string)
          div = self.appyElementSettings(etree.Element('div'), settings)
        """
        for attr in keys:
            if (attr in settings) and (settings[attr] != None):
                element.set(attr, settings[attr])
        return element


    def createErrorElement(self, message, title='Markdown Parsing Error', parent=None, error=True):
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
            error[bool]: Log an error (default: True).
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
            log.error('{}: {}'.format(title, message))
        return el
