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
import logging
import collections
import markdown
import anytree
import mooseutils
import MooseDocs
from . import common

LOG = logging.getLogger(__name__)

class MooseMarkdown(markdown.Markdown):
    """
    A custom Markdown object for handling raw text, markdown files, or MarkdownNode objects.

    The key to this class is allowing the Markdown object to work with MarkdownNode objects such
    that the extension objects, namely MooseTemplate, could have access to the node object to allow
    for searching the tree for other pages. This should allow for cross page figure, equation, and
    table links to be created.

    Args:
        config[dict]: The configure dict, if not provided getDefaultExtensions is used.
        config_file[str]: The name of the configuration file to import, this is applied to the
                          supplied or default 'config'.
    """
    EQUATION_COUNT = 0   # counters for numbered equation numbers
    CACHE = dict() # Cache for find method

    @staticmethod
    def getDefaultExtensions():
        """
        Return an OrderedDict that defines the default configuration for extensions.

        It returns an OrderedDict such that the dict() can serve to populate the list of extensions
        (i.e., the keys) in the desired instantiate order as well as the configuration settings.

        If no settings are needed for the entry simply set the entry to contain an empty dict()

        """
        ext = collections.OrderedDict() # used to maintain insert order

        # http://pythonhosted.org/Markdown/extensions/index.html
        ext['toc'] = dict()
        ext['smarty'] = dict()
        ext['admonition'] = dict()
        ext['extra'] = dict()
        ext['meta'] = dict()

        # MooseDocs
        ext['MooseDocs.extensions.global'] = dict()
        ext['MooseDocs.extensions.include'] = dict()
        ext['MooseDocs.extensions.bibtex'] = dict()
        ext['MooseDocs.extensions.css'] = dict()
        ext['MooseDocs.extensions.devel'] = dict()
        ext['MooseDocs.extensions.misc'] = dict()
        ext['MooseDocs.extensions.media'] = dict()
        ext['MooseDocs.extensions.tables'] = dict()
        ext['MooseDocs.extensions.listings'] = dict()
        ext['MooseDocs.extensions.refs'] = dict()
        ext['MooseDocs.extensions.app_syntax'] = dict()
        ext['MooseDocs.extensions.template'] = dict()
        ext['MooseDocs.extensions.gchart'] = dict()
        ext['MooseDocs.extensions.admonition'] = dict()
        ext['MooseDocs.extensions.katex'] = dict()
        return ext

    def __init__(self, config=None, default=True):

        # Create the default configuration
        ext_config = self.getDefaultExtensions() if default else collections.OrderedDict()

        # Apply the supplied configuration
        if config is not None:
            ext_config.update(config)

        # Define storage for the current MarkdownNode
        self.current = None
        super(MooseMarkdown, self).__init__(extensions=ext_config.keys(),
                                            extension_configs=ext_config)

    def requireExtension(self, required):
        """
        Raise an exception of the supplied extension type is not registered.
        """
        if not self.getExtension(required):
            raise mooseutils.MooseException("The {} extension is required." \
                                            .format(required.__name__))

    def getExtension(self, etype):
        """
        Return an extension instance.

        Args:
            etype[type]: The type of the extension to return.
        """
        out = None
        for ext in self.registeredExtensions:
            if isinstance(ext, etype):
                out = ext
                break
        return out

    def convert(self, md):
        """
        Convert the raw text, markdown file, or node to html.

        Args:
            md[str]: A markdown file, markdown content, or MarkdownNode
        """
        self.EQUATION_COUNT = 0   #pylint: disable=invalid-name
        self.current = None

        if isinstance(md, str):
            self.current = common.nodes.MarkdownNode('', content=md)
        elif isinstance(md, common.nodes.MarkdownFileNodeBase):
            self.current = md
            LOG.debug('Converting %s to html.', self.current.filename) #pylint: disable=no-member
        else:
            raise mooseutils.MooseException("The supplied content must be a markdown str or a "
                                            "MarkdownFileNodeBase object.")

        return super(MooseMarkdown, self).convert(self.current.content)

    def getFilename(self, desired, check_local=True):
        """
        Locate nodes with a filename ending with provided string.
        """
        if check_local:
            local = os.path.join(MooseDocs.ROOT_DIR, desired)
            if os.path.exists(local):
                return local, None

        nodes = self.find(self.current.root, desired)
        if len(nodes) > 1:
            msg = "Multiple filenames matching '{}' found:".format(desired)
            for n in nodes:
                msg += '\n    {}'.format(n.filename)
            LOG.error(msg)

        if nodes:
            return nodes[0].filename, nodes[0]

        return None, None

    @staticmethod
    def find(node, desired):
        """
        Find method for filenames (mainly for testing).
        """
        if desired in MooseMarkdown.CACHE:
            return MooseMarkdown.CACHE[desired]

        types = (common.nodes.FileNodeBase)
        filter_ = lambda n: isinstance(n, types) and n.filename.endswith(desired)
        nodes = [n for n in anytree.iterators.PreOrderIter(node, filter_=filter_)]
        MooseMarkdown.CACHE[desired] = nodes
        return nodes
