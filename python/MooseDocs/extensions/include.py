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

from markdown.preprocessors import Preprocessor
from markdown.util import etree

from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon
from listings import ListingPattern

class IncludeExtension(MooseMarkdownExtension):
    """
    Extension for recursive including of partial or complete markdown files.
    """
    @staticmethod
    def defaultConfig():
        """Default IncludeExtension configuration options."""
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds include support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose-markdown-include',
                             MarkdownPreprocessor(markdown_instance=md, **config), '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create IncludeExtension"""
    return IncludeExtension(*args, **kwargs)

class MarkdownPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    An recursive include command for including a markdown file from within another. This adds the
    ability to specify start/end string to include only portions for the markdown.
    """
    REGEX = r'(?<!^```\n)^!include\s+(.*?)(?:$|\s+)(.*)'

    @staticmethod
    def defaultSettings():
        """Settigns for MarkdownPreprocessor"""
        settings = MooseMarkdownCommon.defaultSettings()
        l_settings = ListingPattern.defaultSettings()
        settings['re'] = (None, "Python regular expression to use for removing text, with flags " \
                                "set to MULTILINE|DOTALL. If a 'markdown' group is set, only the" \
                                "text in that group will be included.")
        settings['start'] = l_settings['start']
        settings['end'] = l_settings['end']
        settings['include-end'] = l_settings['include-end']
        settings['include-start'] = l_settings['include-start']
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        super(MarkdownPreprocessor, self).__init__(markdown_instance=markdown_instance, **kwargs)
        self.markdown = markdown_instance
        self._found = False

    def replace(self, match):
        """
        Substitution function for the re.sub function.
        """
        filename, _ = self.getFilename(match.group(1))
        if not filename:
            msg = "Failed to located filename in following command in file {}.\n    {}"
            el = self.createErrorElement(msg.format(self.markdown.current.filename, match.group(0)),
                                         title="Unknown Markdown File")
            return etree.tostring(el)

        settings = self.getSettings(match.group(2))
        if settings['start'] or settings['end']:
            content = ListingPattern.extractLineRange(filename, settings['start'],
                                                      settings['end'], settings['include-start'],
                                                      settings['include-end'])
        else:
            with open(filename, 'r') as fid:
                content = fid.read()

        if settings['re']:
            match = re.search(settings['re'], content, flags=re.MULTILINE|re.DOTALL)
            if not match:
                msg = "Failed to located regex in following command.\n{}"
                el = self.createErrorElement(msg.format(settings['re']), title="Failed Regex")
                return etree.tostring(el)
            if 'markdown' in match.groupdict():
                content = match.group('markdown')
            else:
                content = match.group(0)

        self._found = True
        return content

    def run(self, lines):
        """
        Recursive markdown replacement.
        """
        content = '\n'.join(lines)
        match = True
        while match:
            self._found = False
            content = re.sub(self.REGEX, self.replace, content, flags=re.MULTILINE)
            if not self._found:
                break
        return content.splitlines()
