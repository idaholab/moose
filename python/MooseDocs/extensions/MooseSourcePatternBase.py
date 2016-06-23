import re
import os
import logging
import copy
log = logging.getLogger(__name__)

from markdown.inlinepatterns import Pattern
from markdown.util import etree
import MooseDocs

class MooseSourcePatternBase(Pattern):
    """
    Base class for pattern matching source code blocks.

    Args:
        regex: The string containing the regular expression to match.
        language[str]: The code language (e.g., 'python' or 'c++')
    """

    def __init__(self, regex, config, language=None):
        Pattern.__init__(self, regex)
        #super(Pattern, self).__init__(regex) # This fails

        # Set the language
        self._language = language

        # The root directory
        self._config = config

        # The default settings
        self._settings = {'strip_header':True,
                          'repo_link':True,
                          'label':True,
                          'overflow-y':'scroll',
                          'max-height':'500px',
                          'strip-extra-newlines':False}

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
                    #@TODO: Log this
                    print 'Unknown setting', k
                    continue
                try:
                    output[k] = eval(v)
                except:
                    output[k] = str(v)
        return output

    def style(self, *keys):
        """
        Extract the html style string from a list of settings.

        Args:
            *keys[str]: A list of keys to compose into a style string.
        """
        style = []
        for k in keys:
            style.append('{}:{}'.format(k, self._settings[k]))
        return ';'.join(style)

    def prepareContent(self, content, settings):
        """
        Prepare the convent for conversion to Element object.

        Args:
            content[str]: The content to prepare (i.e., the file contents).
        """

        # Strip leading/trailing newlines
        content = re.sub(r'^(\n*)', '', content)
        content = re.sub(r'(\n*)$', '', content)

        # Strip extra new lines (optional)
        if settings['strip-extra-newlines']:
            content = re.sub(r'(\n{3,})', '\n\n', content)

        # Strip header and leading/trailing whitespace and newlines
        if self._settings['strip_header']:
            strt = content.find('/********')
            stop = content.rfind('*******/\n')
            content = content.replace(content[strt:stop+9], '')

        return content.strip()


    def createErrorElement(self, rel_filename):
        """
        Returns a tree element containing error message.
        """
        el = etree.Element('p')
        el.set('style', "color:red;font-size:120%")
        el.text = 'ERROR: Invalid markdown for: ' + rel_filename
        return el


    def checkFilename(self, rel_filename):
        """
        Checks that the filename exists, if it does not a error Element is return.

        Args:
            filename[str]: The filename to check for existence.
        """

        filename = os.path.abspath(os.path.join(self._config['root'], rel_filename))
        if os.path.exists(filename):
            return filename
        return None

    def createElement(self, label, content, filename, rel_filename, settings):
        """
        Create the code element from the supplied source code content.

        Args:
            label[str]: The label supplied in the regex, [label](...)
            content[str]: The code content to insert into the markdown.
            filename[str]: The complete filename (for error checking)
            rel_filename[str]: The relative filename; used for creating github link.
            settings[dict]: The current settings.

        NOTE: The code related settings and clean up are applied in this method.
        """

        # Strip extra new lines
        content = self.prepareContent(content, settings)

        # Build outer div container
        el = etree.Element('div')

        # Build label
        if settings['repo_link'] and self._config['repo']:
            title = etree.SubElement(el, 'a')
            title.set('href', os.path.join(self._config['repo'], rel_filename))
        else:
            title = etree.SubElement(el, 'div')

        if self._settings['label']:
            title.text = label

        # Build the code
        pre = etree.SubElement(el, 'pre')
        code = etree.SubElement(pre, 'code')
        if self._language:
            code.set('class', 'hljs ' + self._language)
        code.set('style', self.style('overflow-y', 'max-height'))
        code.text = content

        return el
