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
import re
import collections
import logging

from markdown.util import etree

import MooseDocs

LOG = logging.getLogger(__name__)

class Item(object):
    """
    List item object.

    Args:
      filename[str]: The absolute path to the file.
      repo[str]: The repository url to append the relative path.
    """
    def __init__(self, filename, repo):
        self.filename = filename.replace(MooseDocs.ROOT_DIR, '').lstrip('/')
        self.remote = '{}/{}'.format(repo.rstrip('/'), self.filename)

    def html(self):
        """
        Returns an html li tag.
        """
        el = etree.Element('li')
        a = etree.SubElement(el, 'a')
        a.set('href', self.remote)
        a.text = self.filename
        return el

class MooseLinkDatabase(object):
    """
    Creates a database for creating links to input files and source code.

    Args:
      repo[str]: The GitHub/GitLab url to append filename for creating hyperlink.
      links[dict]: A dictionary of paths to search, with the key being the heading and the
                   value being a list of paths.
    """

    INPUT_RE = re.compile(r'\btype\s*=\s*(?P<key>\w+)\b')
    HEADER_RE = re.compile(r'\bpublic\s+(?P<key>\w+)\b')

    # This object is passed a general config dict() to it may have other arguments.
    def __init__(self, repo=None, links=None):
        self._repo = repo
        self.inputs = collections.OrderedDict()
        self.children = collections.OrderedDict()

        for key, paths in links.iteritems():
            self.inputs[key] = dict()
            self.children[key] = dict()

            for path in paths:
                for base, _, files in os.walk(os.path.join(MooseDocs.ROOT_DIR, path)):
                    for filename in files:
                        full_name = os.path.join(base, filename)
                        if filename.endswith('.i'):
                            self.search(full_name, self.INPUT_RE, self.inputs[key])
                        elif filename.endswith('.h'):
                            self.search(full_name, self.HEADER_RE, self.children[key])

    def search(self, filename, regex, database):
        """
        Search the file and create list item if found.

        Args:
          filename[str]: The filename to open for searching.
          regex: The compiled re to use for searching file, it must contain a "key" group for
                 building database.
          database: The database (dict) to append if the key is located.
        """
        match = False
        with open(filename, 'r') as fid:
            for match in re.finditer(regex, fid.read()):
                key = match.group('key')
                if key not in database:
                    database[key] = set()
                database[key].add(Item(filename, self._repo))
