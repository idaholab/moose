import re
import os
from markdown.util import etree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseSourcePatternBase import MooseSourcePatternBase
import utils

try:
    import utils.MooseSourceParser
    HAVE_MOOSE_CPP_PARSER = True
except:
    HAVE_MOOSE_CPP_PARSER = False


class MooseCppMethod(MooseSourcePatternBase):
    """
    A markdown extension for including complete source code files.
    """

    # REGEX for finding: [Something here](source.C::method_name)
    CPP_RE = r'!\[(.*?)\]\((.*\.[Ch])::(\w+)\s*(.*?)\)'

    def __init__(self, make=None, **kwargs):
        super(MooseCppMethod, self).__init__(self.CPP_RE, language='cpp', **kwargs)

        # The make command to execute
        self._make = make

    def handleMatch(self, match):
        """
        Process the C++ file provided.
        """
        # Update the settings from regex match
        settings = self.getSettings(match.group(5))

        # Extract relative filename
        rel_filename = match.group(3).lstrip('/')

        # Error if the clang parser did not load
        if not HAVE_MOOSE_CPP_PARSER:
            log.error("Unable to load the MOOSE clang C++ parser.")
            el = self.createErrorElement(rel_filename, message="Failed to load python clang python bindings.")
            return el

        # Read the file and create element
        filename = self.checkFilename(rel_filename)
        if not filename:
            el = self.createErrorElement(rel_filename)
        else:
            if self._make == None:
                log.error('The location of the Makefile must be supplied to parser.')
                el = self.createErrorElement(rel_filename)
            else:
                log.info('Parsing method "{}" from {}'.format(match.group(4), filename))

                parser = utils.MooseSourceParser(self._make)
                parser.parse(filename)
                decl, defn = parser.method(match.group(4))
                el = self.createElement(match.group(2), defn, filename, rel_filename, settings)

        # Return the Element object
        return el
