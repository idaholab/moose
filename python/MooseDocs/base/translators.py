#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Module that defines Translator objects for converted AST from Reader to Rendered output from
Renderer objects. The Translator objects exist as a place to import extensions and bridge
between the reading and rendering content.
"""
import os
import logging
import multiprocessing
import types

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import mixins, exceptions
from MooseDocs.tree import pages
from .components import Extension
from .readers import Reader
from .renderers import Renderer
from .executioners import ParallelBarrier

LOG = logging.getLogger('MooseDocs.Translator')

class Translator(mixins.ConfigObject):
    """
    Object responsible for converting reader content into an AST and rendering with the
    supplied renderer.

    Inputs:
        content[page.Page]: A tree of input "pages".
        reader[Reader]: A Reader instance.
        renderer[Renderer]: A Renderer instance.
        extensions[list]: A list of extensions objects to use.
        kwargs[dict]: Key, value pairs applied to the configuration options.

    This class is the workhorse of MOOSEDocs, it is the hub for all data in and out.  It is not
    designed to be customized and extensions have no access to this the class.
    """
    #: A multiprocessing lock. This is used in various locations, mainly prior to caching items
    #  as well as during directory creation.
    LOCK = multiprocessing.Lock()

    #: A code for indicating that parallel work is done
    PROCESS_FINISHED = -1

    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['profile'] = (False, "Perform profiling of tokenization and rendering, " \
                                    "this runs in serial.")
        config['destination'] = (os.path.join(os.getenv('HOME'), '.local', 'share', 'moose',
                                              'site'),
                                 "The output directory.")
        config['number_of_suggestions'] = (5, "The number of page names to suggest when a file " \
                                              "cannot be found.")
        return config

    def __init__(self, content, reader, renderer, extensions, executioner=None, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)

        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('content', content, pages.Page)
            common.check_type('reader', reader, Reader)
            common.check_type('renderer', renderer, Renderer)
            common.check_type('extensions', extensions, list)
            for ext in extensions:
                common.check_type('extensions', ext, Extension)

        self.__initialized = False
        self.__content = content
        self.__extensions = extensions
        self.__reader = reader
        self.__renderer = renderer

        # Define an Executioner if not provided
        self.__executioner = executioner
        if executioner is None:
            self.__executioner = ParallelBarrier()

        # Caching for page searches (see findPages)
        self.__page_cache = dict()

        # Cache for looking up markdown files for levenshtein distance
        self.__markdown_file_list = None
        self.__levenshtein_cache = dict()

    @property
    def extensions(self):
        """Return list of loaded Extension objects."""
        return self.__extensions

    @property
    def reader(self):
        """Return the Reader object."""
        return self.__reader

    @property
    def renderer(self):
        """Return the Renderer object."""
        return self.__renderer

    @property
    def content(self):
        """Return the content."""
        return self.__content

    @property
    def executioner(self):
        """Return the Executioner instance."""
        return self.__executioner

    @property
    def destination(self):
        """Return the destination directory."""
        return self.get('destination')

    def addContent(self, page):
        """Add an additional page to the list of available pages."""
        self.__content.append(page)

    def update(self, **kwargs):
        """Update configuration and handle destination."""
        dest = kwargs.get('destination', None)
        if dest is not None:
            kwargs['destination'] = mooseutils.eval_path(dest)
        mixins.ConfigObject.update(self, **kwargs)

    def updateConfiguration(self, obj_name, **kwargs):
        """Update configuration from meta data."""
        if obj_name == 'reader':
            self.__reader.update(error_on_unknown=False, **kwargs)
        elif obj_name == 'renderer':
            self.__renderer.update(error_on_unknown=False, **kwargs)
        else:
            for ext in self.__extensions:
                if ext.name == obj_name:
                    ext.update(error_on_unknown=False, **kwargs)

    def resetConfigurations(self):
        """Reset configuration to original state."""
        self.__reader.resetConfig()
        self.__renderer.resetConfig()
        for ext in self.extensions:
            ext.resetConfig()

    def getMetaData(self, page, key):
        """Return the Meta data for the supplied page and key."""
        return self.__executioner.getMetaData(page, key)

    def getMetaDataObject(self, page):
        """Return the Meta data object for the supplied page."""
        return self.__executioner.getMetaDataObject(page)

    def getSyntaxTree(self, page):
        """Return the AST for the supplied page."""
        return self.__executioner.getSyntaxTree(page)

    def getResultTree(self, page):
        """Return the rendered tree for the supplied page."""
        return self.__executioner.getResultTree(page)

    def findPages(self, arg, exact=False):
        """
        Locate all Page objects that operates on a string or uses a filter.

        Usage:
           nodes = self.findPages('name')
           nodes = self.findPages(lambda p: p.name == 'foo')

        Inputs:
            name[str|str|lambda]: The partial name to search against or the function to use
                                      to test for matches.
            exact[bool]: (False) When True an exact path match is required.
        """
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('name', arg, (str, str, types.FunctionType))

        if isinstance(arg, (str, str)):
            items = self.__page_cache.get(arg, None)
            if items is None:
                func = lambda p: (p.local == arg) or \
                                 (not exact and p.local.endswith(os.sep + arg.lstrip(os.sep)))
                items = [page for page in self.__content if func(page)]
                self.__page_cache[arg] = items

        else:
            items = [page for page in self.__content if arg(page)]

        return items

    def findPage(self, arg, throw_on_zero=True, exact=False, warn_on_zero=False):
        """
        Locate a single Page object that has a local name ending with the supplied name.

        Inputs:
            see findPages
        """
        nodes = self.findPages(arg, exact)
        if len(nodes) == 0:
            if throw_on_zero or warn_on_zero:
                msg = "Unable to locate a page that ends with the name '{}'.".format(arg)
                num = self.get('number_of_suggestions', 0)
                if num:
                    if self.__markdown_file_list is None:
                        self.__buildMarkdownFileCache()

                    dist = self.__levenshtein_cache.get(arg, None)
                    if dist is None:
                        dist = mooseutils.levenshteinDistance(arg, self.__markdown_file_list,
                                                              number=num)
                        self.__levenshtein_cache[arg] = dist
                    msg += " Did you mean one of the following:\n"
                    for d in dist:
                        msg += "     {}\n".format(d)

                if warn_on_zero:
                    LOG.warning(msg)
                    return None
                else:
                    raise exceptions.MooseDocsException(msg)
            else:
                return None

        elif len(nodes) > 1:
            msg = "Multiple pages with a name that ends with '{}' were found:".format(arg)
            for node in nodes:
                msg += '\n  {}'.format(node.local)
            raise exceptions.MooseDocsException(msg)
        return nodes[0]

    def __buildMarkdownFileCache(self):
        """Builds a list of markdown files, including the short-hand version for error reports."""

        self.__markdown_file_list = set()
        for local in [page.local for page in self.__content if isinstance(page, pages.Source)]:
            self.__markdown_file_list.add(local)
            parts = local.split(os.path.sep)
            n = len(parts)
            for i in xrange(n, 0, -1):
                self.__markdown_file_list.add(os.path.join(*parts[n-i:n]))

    def init(self):
        """
        Initialize the translator with the output destination for the converted content.

        This method also initializes all the various items within the translator for performing
        the conversion. It is required to allow the build command to modify configuration items
        (i.e., the 'destination' option) prior to setting up the extensions.

        Inputs:
            destination[str]: The path to the output directory.
        """
        if self.__initialized:
            msg = "The {} object has already been initialized, this method should not " \
                  "be called twice."
            raise MooseDocs.common.exceptions.MooseDocsException(msg, type(self))

        # Attach translator to Executioner
        self.__executioner.setTranslator(self)

        # Initialize the extension and call the extend method, then set the extension object
        # on each of the extensions.
        destination = self.get("destination")
        for ext in self.__extensions:
            common.check_type('extensions', ext, MooseDocs.base.components.Extension)
            ext.setTranslator(self)
            ext.extend(self.__reader, self.__renderer)
            for comp in self.__reader.components:
                if comp.extension is None:
                    comp.setExtension(ext)

            for comp in self.__renderer.components:
                if comp.extension is None:
                    comp.setExtension(ext)

        # Set the translator
        for comp in self.__reader.components:
            comp.setTranslator(self)
        for comp in self.__renderer.components:
            comp.setTranslator(self)

        # Check that the extension requirements are met
        for ext in self.__extensions:
            self.__checkRequires(ext)

        for node in self.__content:
            node.base = destination
            if isinstance(node, pages.Source):
                node.output_extension = self.__renderer.EXTENSION

        self.__initialized = True

    def execute(self, num_threads=1, nodes=None):
        """Perform build for all pages, see executioners."""
        self.__assertInitialize()
        self.__executioner(nodes, num_threads)

    def __assertInitialize(self):
        """Helper for asserting initialize status."""
        if not self.__initialized:
            msg = "The Translator.init() method must be called prior to executing this method."
            raise exceptions.MooseDocsException(msg)

    def __checkRequires(self, extension):
        """Helper to check the loaded extensions."""
        available = [e.__module__ for e in self.__extensions]
        messages = []
        for ext in extension._Extension__requires: #pylint: disable=protected-access
            if ext.__name__ not in available:
                msg = "The {} extension is required but not included.".format(ext.__name__)
                messages.append(msg)

        if messages:
            raise exceptions.MooseDocsException('\n'.join(messages))
