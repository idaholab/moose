#pylint: disable=missing-docstring,no-member
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
import codecs
import types
import urlparse

import anytree

import MooseDocs
from MooseDocs import common
from MooseDocs.common import exceptions, mixins
from MooseDocs.tree import base, tokens

LOG = logging.getLogger(__name__)
CACHE = dict() # Creates a global cache for faster searching, anytree search is very slow

class PageNodeBase(base.NodeBase, mixins.TranslatorObject):
    """
    Base class for content tree.

    TODO: combine with LocationNodeBase, I don't remember why the two classes.
    """

    PROPERTIES = [base.Property('source', ptype=str)]
    COLOR = None

    def __init__(self, *args, **kwargs):
        mixins.TranslatorObject.__init__(self)
        base.NodeBase.__init__(self, *args, **kwargs)

    def build(self):
        """Performs a 'build', this is called by Translator."""
        self.write()

    def write(self):
        """Write the file to the destination, see LocationNodeBase."""
        pass

class LocationNodeBase(PageNodeBase):
    """
    Base class for locations (Directories and Files).
    """
    PROPERTIES = [base.Property('base', ptype=str, default='')]

    def __init__(self, *args, **kwargs):
        PageNodeBase.__init__(self, *args, **kwargs)
        self.extension = None

        _, self.extension = os.path.splitext(self.source)
        self.name = os.path.basename(self.source)

        self.fullpath = os.path.join(self.parent.fullpath, self.name) if self.parent else self.name

        CACHE[self.fullpath] = set([self])

    @property
    def local(self):
        """Returns the local directory/filename."""
        return self.fullpath

    @property
    def destination(self):
        """Returns the translator destination location."""
        return os.path.join(self.base, self.local)

    def write(self):
        """
        Creates directories that do not exist within the destination. This handles locking with
        multithreading package to avoid race condition on directory creation.
        """
        with self.translator.lock:
            dirname = os.path.dirname(self.destination)
            if dirname and not os.path.isdir(dirname):
                os.makedirs(dirname)

    def findall(self, name, maxcount=1, mincount=1, exc=exceptions.MooseDocsException): #pylint: disable=no-self-use
        """
        Find method for locating pages.

        Inputs:
            name[str]: The name of the page to search.
            maxcount[int]: The maximum number of items to find (default: 1).
            mincount[int]: The minimum number of items to find (default: 1).
            exc[Exception]: The type of exception to raise if min/max are not satisfied.
        """

        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('name', name, (str, unicode))
            common.check_type('mincount', mincount, (int))
            common.check_type('maxcount', maxcount, (int))
            common.check_type('exc', exc, (type, types.LambdaType, type(None)))

        try:
            return list(CACHE[name])

        except KeyError:
            pass

        nodes = set()
        for key in CACHE:
            if key.endswith(name):
                nodes.update(CACHE[key])

        if (maxcount is not None) and exc and (len(nodes) > maxcount):
            msg = "The 'maxcount' was set to {} but {} nodes were found for the name '{}'." \
                  .format(maxcount, len(nodes), name)
            for node in nodes:
                msg += '\n  {} (source: {})'.format(node.local, node.source)
            raise exc(msg)

        elif (mincount is not None) and exc and (len(nodes) < mincount):
            msg = "The 'mincount' was set to {} but {} nodes were found for the name '{}'." \
                  .format(mincount, len(nodes), name)
            for node in nodes:
                msg += '\n  {} (source: {})'.format(node.local, node.source)
            raise exc(msg)

        CACHE[name] = nodes
        return list(nodes)

    def relativeSource(self, other):
        """ Location of this page related to the other page."""
        return os.path.relpath(self.local, os.path.dirname(other.local))

    def relativeDestination(self, other):
        """
        Location of this page related to the other page.

        Inputs:
            other[LocationNodeBase]: The page that this page is relative too.
        """
        return os.path.relpath(self.destination, os.path.dirname(other.destination))


    def console(self):
        """Define the anytree screen output."""
        return '{} ({}): {}, {}'.format(self.name, self.__class__.__name__, self.local, self.source)

class DirectoryNode(LocationNodeBase):
    """
    Directory nodes.
    """
    COLOR = 'CYAN'

class FileNode(LocationNodeBase):
    """
    File nodes.
    """
    COLOR = 'MAGENTA'

    def write(self):
        LocationNodeBase.write(self)
        LOG.debug('COPY: %s-->%s', self.source, self.destination)
        shutil.copyfile(self.source, self.destination)

class MarkdownNode(FileNode):
    """
    Node for content to be converted via Translator.

    This object handles cache of the content so when the Translator calls multiple builds it doesn't
    rebuild all the content.

    #TODO: Test the re-build cache.
    #TODO: Re-name this to TranslateNode and get the extensions from Reader/Renderer objects.
    """

    PROPERTIES = [base.Property('content', ptype=unicode)]

    def __init__(self, *args, **kwargs):
        FileNode.__init__(self, *args, **kwargs)

        self._modified = 0
        if self.source and os.path.exists(self.source):
            self._modified = os.path.getmtime(self.source)

        self._ast = None
        self._result = None
        self._index = None

    @property
    def destination(self):
        """The content destination (override)."""
        return super(MarkdownNode, self).destination.replace('.md',
                                                             self.translator.renderer.EXTENSION)

    @property
    def ast(self):
        """Return the current AST."""
        return self._ast

    @property
    def result(self):
        """Return the rendered result."""
        return self._result

    @property
    def index(self):
        """Return the index."""
        return self._index

    def tokenize(self):
        """
        Perform tokenization of content, using cache if the content has not changed.
        """
        if self.modified() or (self.content is None):
            self._ast = None
            self._result = None
            self.read()

        if self._ast is None:
            self._ast = tokens.Token(None)
            self.translator.reader.parse(self._ast, self.content)

        return self._ast

    def render(self, ast):
        """
        Render supplied tokens to the output format.
        """
        if self._result is None:
            self._result = self.translator.renderer.render(ast)
        return self._result

    def read(self):
        """
        Read the content for conversion.
        """
        if self.source and os.path.exists(self.source):
            LOG.debug('READ %s', self.source)
            self._modified = os.path.getmtime(self.source)
            self.content = common.read(self.source).lstrip('\n') #pylint: disable=attribute-defined-outside-init

    def modified(self):
        """
        Returns True if the content has been modified from the last call.
        """
        if self.source and os.path.exists(self.source):
            return os.path.getmtime(self.source) > self._modified
        return True

    def write(self):
        """
        Write the converted text to the output destination.
        """
        if self._result is not None:
            LOG.debug('WRITE %s -> %s', self.source, self.destination)
            LocationNodeBase.write(self) # Creates directories
            with codecs.open(self.destination, 'w', encoding='utf-8') as fid:
                fid.write(self._result.write())

    def buildIndex(self, home):
        """
        Build the search index.
        """
        if (self._index is None) and (self._result is not None):
            self._index = []
            for section in anytree.search.findall_by_attr(self._result, 'section'):
                name = self.name.replace('_', ' ')
                if name.endswith('.md'):
                    name = name[:-3]
                text = section['data-section-text']
                location = urlparse.urlsplit(self.destination.replace(self.base, home)) \
                    ._replace(scheme=None, netloc=None, fragment=str(section['id'])).geturl()
                self._index.append(dict(name=name, text=text, location=location))

    def build(self):
        """
        Build method for livereload.
        """
        self.translator.current = self
        self.translator.reinit()
        ast = self.tokenize()
        self.render(ast)
        self.write()
        self.translator.current = None
