import os
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from markdown.inlinepatterns import Pattern
from MooseCommonExtension import MooseCommonExtension

class MoosePackageParser(MooseCommonExtension, Pattern):
    """
    Markdown extension for extracting package arch and version.
    """

    RE = r'!MOOSEPACKAGE\s*(.*?)!'

    def __init__(self, markdown_instance=None, **kwargs):
        MooseCommonExtension.__init__(self, **kwargs)
        Pattern.__init__(self, self.RE, markdown_instance)

        # Load the yaml data containing package information
        self.package = MooseDocs.yaml_load(os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'packages.yml'))

        # The default settings
        self._settings['arch'] = None
        self._settings['return'] =  None

    def handleMatch(self, match):
        """
        Returns a tree element package information.
        """
        # Update the settings from regex match
        settings = self.getSettings(match.group(2))
        if not settings.has_key('arch') or not settings.has_key('return'):
            el = self.createErrorElement('Invalid MOOSEPACKAGE markdown syntax. Requires arch=, return=link|name')
        else:
            if settings['arch'] not in self.package.keys():
                el = self.createErrorElement('"arch" not found in packages.yml')
            else:
                if settings['return'] == 'link':
                    el = etree.Element('a')
                    el.set('href', self.package['link'] + self.package[settings['arch']]['name'])
                    el.text = self.package[settings['arch']]['name']
                elif settings['return'] == 'name':
                    el = etree.Element('p')
                    el.text = self.package[settings['arch']]['name']
        return el
