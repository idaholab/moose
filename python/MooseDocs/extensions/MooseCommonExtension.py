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

    def getSettings(self, settings):
        """
        Return the settings captured from the regular expression.

        Args:
        settings[str]: A string containing the space separate key, value pairs (key=value key2=value2).
        """
        output = copy.copy(self._settings)
        for s in settings.split(' '):
            if s:
                k, v = s.strip().split('=')
                if k not in output:
                    log.warning('Unknown setting {}'.format(k))
                    continue
                try:
                    output[k] = eval(v)
                except:
                    output[k] = str(v)
        return output

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
