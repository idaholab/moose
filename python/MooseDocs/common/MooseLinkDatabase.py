#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
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
      filename[str]: The absoulte path to the file.
      repo[str]: The repository url to append the relative path.
    """
    def __init__(self, filename, repo):
        self.filename = MooseDocs.relpath(filename).lstrip('/')
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
      root[str]: The repository root directory.
      repo[str]: The GitHub/GitLab url to append filename for creating hyperlink.
      links[dict]: A dictionary of paths to search, with the key being the heading and the
                   value being a list of paths.
    """

    INPUT_RE = re.compile(r'\btype\s*=\s*(?P<key>\w+)\b')
    HEADER_RE = re.compile(r'\bpublic\s+(?P<key>\w+)\b')

    # This object is passed a general config dict() to it may have other arguments.
    def __init__(self, repo=None, links=None, **kwargs): #pylint: disable=unused-argument
        self._repo = repo
        self.inputs = collections.OrderedDict()
        self.children = collections.OrderedDict()

        for key, paths in links.iteritems():
            self.inputs[key] = dict()
            self.children[key] = dict()

            for path in paths:
                for base, _, files in os.walk(MooseDocs.abspath(path), topdown=False):
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
                    database[key] = []
                database[key].append(Item(filename, self._repo))
