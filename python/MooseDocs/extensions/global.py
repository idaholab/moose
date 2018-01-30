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

import os
from markdown.preprocessors import Preprocessor
import MooseDocs
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

class GlobalExtension(MooseMarkdownExtension):
    """
    Extension for adding global markdown style link ids.
    """
    @staticmethod
    def defaultConfig():
        """Default GlobalExtension configuration options"""
        config = MooseMarkdownExtension.defaultConfig()
        config['globals'] = [dict(), "A dict of global markdown links (e.g., [foo]: bar)."]
        config['import'] = [[], "List of *.yml files to import and append into global lists."]
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds Bibtex support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()
        md.preprocessors.add('moose_globals',
                             GlobalPreprocessor(markdown_instance=md, **config), '_begin')

def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create GlobalExtension"""
    return GlobalExtension(*args, **kwargs)

class GlobalPreprocessor(MooseMarkdownCommon, Preprocessor):
    """
    Appends global links to markdown content
    """
    @staticmethod
    def defaultSettings():
        """GlobalPreprocessor settings"""
        return dict() # this extension doesn't have settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        Preprocessor.__init__(self, markdown_instance)

        # Import the directly defined globals
        self._globals = kwargs.pop('globals', dict())

        # Add the defined globals from the supplied file(s)
        for filename in kwargs.pop('import'):
            self._globals.update(MooseDocs.yaml_load(os.path.join(MooseDocs.ROOT_DIR, filename)))

    def run(self, lines):
        """
        Append globals to the markdown lines.
        """
        return lines + ['[{}]: {}'.format(key, value) for key, value in self._globals.iteritems()]
