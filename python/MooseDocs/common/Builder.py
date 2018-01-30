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
import shutil
import logging
import multiprocessing
import anytree
import mooseutils
from . import nodes

LOG = logging.getLogger(__name__)

class Builder(object):
    """
    Object for building html from markdown.
    """
    def __init__(self, parser=None, site_dir=None):
        self._site_dir = site_dir # location for the html files
        self._parser = parser     # MooseMarkdown parser
        self._root = None         # the root node (see buildNodes)
        self._lock = multiprocessing.Lock() # lock for threaded building

    def __iter__(self):
        """
        Allow direct iteration over pages (MarkdownFileNodeBase nodes) contained in this object.
        """
        if self._root is None:
            raise mooseutils.MooseException("The 'init' method must be called prior to iterating "
                                            "over the 'nodes' objects")

        filter_ = lambda n: isinstance(n, nodes.MarkdownFileNodeBase)
        for node in anytree.iterators.PreOrderIter(self._root, filter_=filter_):
            yield node

    def __str__(self):
        """
        When print is called on this object the tree structure will display.
        """
        return str(self._root)

    def count(self):
        """
        Return the number of pages.
        """
        if self._root is None:
            raise mooseutils.MooseException("The 'init' method must be called prior to count.")
        count = 0
        for node in self._root.descendants:
            if isinstance(node, nodes.CopyFileNode) and (node.name.endswith('.moose.svg')):
                continue
            count += 1
        return count

    def init(self):
        """
        Initialize the pages.
        """
        self._root = self.buildNodes()

    def build(self, num_threads=multiprocessing.cpu_count()):
        """
        Build all the pages in parallel.
        """
        if self._root is None:
            raise mooseutils.MooseException("The 'init' method must be called prior to build.")

        # Build the complete markdown file tree from the configuration supplied at construction
        if not isinstance(self._root, anytree.NodeMixin):
            raise TypeError("The 'buildNodes' method must return a anytree.NodeMixin object.")

        # Build the pages
        pages = list(self)
        jobs = []
        for chunk in mooseutils.make_chunks(pages, num_threads):
            p = multiprocessing.Process(target=self.buildPages, args=(chunk,))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

        self.copyFiles()

    def buildPages(self, pages):
        """
        Loops through supplied pages and call build method.
        """
        for page in pages:
            self.buildPage(page)

    def buildPage(self, page):
        """
        Converts and writes the provided page.
        """
        if isinstance(page, nodes.MarkdownFileNodeBase):
            LOG.debug('Building page: %s', page.filename)
            page.reset()
            html = self.convert(page)
            self.write(page, html)

    def copyFiles(self):
        """
        When the build is complete this method is called to include any necessary files with the
        html build (e.g., media files).
        """
        if self._root:
            for page in self._root.descendants:
                if isinstance(page, nodes.CopyFileNode):
                    destination = os.path.join(self._site_dir, page.destination)
                    LOG.debug('Copying file: %s -> %s', page.filename, destination)
                    if not os.path.exists(os.path.dirname(destination)):
                        os.makedirs(os.path.dirname(destination))
                    shutil.copyfile(page.filename, destination)

    def convert(self, page):
        """
        Converts the supplied page to html.
        """
        try:
            return self._parser.convert(page)
        except UnicodeDecodeError as e:
            LOG.error("Encountered a problem with a unicode character in %s", page.filename)
            raise e

    def write(self, page, content):
        """
        Write the supplied content to the html file.
        """

        # Define the destination
        destination = os.path.join(self._site_dir, page.destination)

        # Make sure the destination directory exists, if it already does do nothing. If it does not
        # exist try to create it, but with a multiprocessing lock so there isn't a race condition.
        with self._lock:
            if not os.path.exists(os.path.dirname(destination)):
                os.makedirs(os.path.dirname(destination))

        # Write the file
        with open(destination, 'w') as fid:
            LOG.debug('Creating %s', destination)
            fid.write(content.encode('utf-8'))

    def buildNodes(self):
        """
        This method should return a anytree tree structure, any node that is a MarkdownFileNodeBase
        will be converted to html, in parallel.
        """
        raise NotImplementedError("The 'buildNodes' method must be defined to return a tree of "
                                  "node objects, from which any MarkdownFileNodeBase nodes are "
                                  "converted to html.")
