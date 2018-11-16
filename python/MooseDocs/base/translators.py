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
import time
import random
import multiprocessing
import types
import traceback

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import mixins, exceptions
from MooseDocs.tree import pages
from components import Extension
from readers import Reader
from renderers import Renderer

LOG = logging.getLogger('MooseDocs.Translator')

class Meta(object):
    """
    Config data object for data on the pages.Page objects.

    The primary purpose for this object is to enable the ability to modify configurations of the
    reader, renderer, and extension objects from within an extension (i.e., the config extension).
    """
    def __init__(self, extensions):
        self.__data = dict(dependencies=set(), reader=dict(), renderer=dict())
        for ext in extensions:
            self.__data[ext.__class__.__name__] = dict()

    def initData(self, key, dtype):
        """Initialize dict key with the supplied type."""
        self.__data[key] = dtype()

    def getData(self, key):
        """Retrieve data for the supplied key."""
        return self.__data[key]

class Barrier(object):
    """
    Implementation of a barrier.
    https://github.com/ghcollin/multitables/blob/master/multitables.py

    NOTE: The multiprocessing package includes a Barrier object as of python 3.3.
    """

    def __init__(self, n_procs):
        """
        Create a barrier that waits for n_procs processes.
        :param n_procs: The number of processes to wait for.
        """
        self.__n_procs = n_procs
        self.__count = multiprocessing.Value('i', 0, lock=False)
        self.__cvar = multiprocessing.Condition()

    def wait(self):
        """Wait until all processes have reached the barrier."""
        with self.__cvar:
            self.__count.value += 1
            self.__cvar.notify_all()
            while self.__count.value < self.__n_procs:
                self.__cvar.wait()

    def reset(self):
        """Re-set the barrier so that it can be used again."""
        with self.__cvar:
            self.__count.value = 0

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
        config['destination'] = (os.path.join(os.getenv('HOME'), '.local', 'share', 'moose',
                                              'site'),
                                 "The output directory.")
        return config

    def __init__(self, content, reader, renderer, extensions, **kwargs):
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
        self.__destination = None # assigned during init()

        # Members used during conversion, see execute
        self.__manager = multiprocessing.Manager()
        self.__page_meta_data = self.__manager.dict()
        self.__page_syntax_trees = self.__manager.dict()
        self.__ast_available = self.__manager.Value('i', 1) # TODO: implement this check

        # Caching for page searches (see findPages)
        self.__page_cache = dict()

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

    def update(self, **kwargs):
        """Update configuration and handle destination."""
        dest = kwargs.get('destination', None)
        if dest is not None:
            kwargs['destination'] = mooseutils.eval_path(dest)
        mixins.ConfigObject.update(self, **kwargs)

    def getMetaData(self, page):
        """Return the Meta data object for the supplied page."""
        if self.__ast_available.value == 0:
            msg = "The meta data for {} page is not available until tokenization is complete, " \
                  "therefore this method should not be used within the pre/postTokenize or " \
                  "createToken methods."
            raise exceptions.MooseDocsException(msg, page.local)

        return self.__page_meta_data.get(page.uid, None)

    def getSyntaxTree(self, page):
        """
        Return the AST for the supplied page, this is used by the RenderComponent.

        see Translator::execute and RenderComponent::setTranslator/getSyntaxTree
        """
        if self.__ast_available.value == 0:
            msg = "The AST data for {} page is not available until tokenization is complete, " \
                  "therefore this method should not be used within the pre/postTokenize or " \
                  "createToken methods."
            raise exceptions.MooseDocsException(msg, page.local)

        elif not isinstance(page, pages.Source):
            msg = "AST data is only available for pages.Source objects; however, a {} object " \
                  "was provided."
            raise exceptions.MooseDocsException(msg, page.__class__.__name__)

        return self.__page_syntax_trees.get(page.uid, None)

    def findPages(self, arg):
        """
        Locate all Page objects that operates on a string or uses a filter.

        Usage:
           nodes = self.findPages('name')
           nodes = self.findPages(lambda p: p.name == 'foo')

        The string version is equivalent to:
           nodes = self.findPages(lambda p: p.local.endswith(arg))

        Inputs:
            name[str|unicode|lambda]: The partial name to search against or the function to use
                                      to test for matches.
        """
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('name', arg, (str, unicode, types.FunctionType))

        if isinstance(arg, (str, unicode)):
            items = self.__page_cache.get(arg, None)
            if items is None:
                func = lambda p: p.local.endswith(arg)
                items = [page for page in self.__content if func(page)]
                self.__page_cache[arg] = items

        else:
            items = [page for page in self.__content if arg(page)]

        return items

    def findPage(self, arg):
        """
        Locate a single Page object that has a local name ending with the supplied name.

        Inputs:
            see findPages
        """
        nodes = self.findPages(arg)
        if len(nodes) == 0:
            msg = "Unable to locate a page that ends with the name '{}'.".format(arg)
            raise exceptions.MooseDocsException(msg)

        elif len(nodes) > 1:
            msg = "Multiple pages with a name that ends with '{}' were found:".format(arg)
            for node in nodes:
                msg += '\n  {} (source: {})'.format(node.local, node.source)
            raise exceptions.MooseDocsException(msg)
        return nodes[0]

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
        """
        Perform parallel build for all pages.

        Inputs:
            num_threads[int]: The number of threads to use (default: 1).
            nodes[list]: A list of Page object to build, if not provided all pages will be created.

        The translator execute method is responsible for "converting" all pages.Page objects
        contained within the self.__root member variable, these shall be referred to as "pages"
        herein. The conversion process is as follows.

        1. Execute preExecute methods
        2. Convert pages.Source pages on multiprocessing processes
           2.1. Read content from page.Source files
           2.2. Execute postRead methods
           2.3. Create the root AST object via Reader::getRoot()
           2.4. Execute preTokenize methods
           2.5. Perform tokenization
           2.6. Execute postTokenize methods

           Parallel Barrier, the AST and Meta data for all pages is available for following steps

           2.7. Create the root result object via Renderer::getRoot()
           2.8. Execute preRender methods
           2.9. Render the AST
           2.10. Execute postRender methods
           2.11. Write the results
        3. Finalize the content, i.e., copy pages.File objects to the destination
        4. Execute postExecute methods
        """
        common.check_type('num_threads', num_threads, int)
        self.__assertInitialize()

        # Extract the Source objects for translation
        nodes = nodes or self.__content
        source_nodes = [n for n in nodes if isinstance(n, pages.Source)]
        other_nodes = [n for n in nodes if not isinstance(n, pages.Source)]

        # Shuffle to improve load balance
        random.shuffle(source_nodes)

        # preExecute
        LOG.info('Executing preExecute methods...')
        t = time.time()
        self.__executeExtensionFunction('preExecute', None, args=(self.__content,))
        LOG.info('Finished preExecute methods [%s sec]', time.time() - t)

        # Translation
        LOG.info('Translating using %s threads...', num_threads)
        t = self.__build(source_nodes, num_threads)
        #t = None; mooseutils.run_profile(self.__build_target, source_nodes)
        LOG.info('Translating complete [%s sec.]', t)

        # Indexing/copying
        LOG.info('Finalizing content...')
        t = self.__finalize(other_nodes, num_threads)
        LOG.info('Finalizing Finished [%s sec.]', t)

        LOG.info('Executing postExecute methods...')
        t = time.time()
        self.__executeExtensionFunction('postExecute', None, args=(self.__content,))
        LOG.info('Finished postExecute methods [%s sec]', time.time() - t)

    def __build(self, source_nodes, num_threads):
        """Translate content from page.Source objects (see execute)."""
        t = time.time()

        if num_threads > len(source_nodes):
            num_threads = len(source_nodes)

        barrier = Barrier(num_threads)

        jobs = []
        for chunk in mooseutils.make_chunks(source_nodes, num_threads):
            p = multiprocessing.Process(target=self.__build_target,
                                        args=(chunk, barrier))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

        return time.time() - t

    def __build_target(self, nodes, barrier):
        """Tokenize and render supplied nodes (see __build)."""

        # Read content and initialize meta data
        for node in nodes:
            content = self.reader.read(node)
            meta = Meta(self.__extensions)
            self.__executeExtensionFunction('postRead', node, args=(content, node, meta))
            self.__updateConfigurations(meta)
            ast = self.reader.getRoot()
            self.__callFunction(self.__reader, 'preTokenize', node, args=(ast, node, meta))
            self.__executeExtensionFunction('preTokenize', node,
                                            args=(ast, node, meta, self.__reader))
            self.reader.tokenize(ast, content, node)
            self.__callFunction(self.__reader, 'postTokenize', node, args=(ast, node, meta))
            self.__executeExtensionFunction('postTokenize', node,
                                            args=(ast, node, meta, self.__reader))
            self.__resetConfigurations()

            self.__page_meta_data[node.uid] = meta
            self.__page_syntax_trees[node.uid] = ast

        barrier.wait()
        barrier.reset()

        # Render
        for node in nodes:
            ast = self.__page_syntax_trees[node.uid]
            meta = self.__page_meta_data[node.uid]
            self.__updateConfigurations(meta)
            result = self.renderer.getRoot()
            self.__callFunction(self.__renderer, 'preRender', node, args=(result, node, meta))
            self.__executeExtensionFunction('preRender', node,
                                            args=(result, node, meta, self.__renderer))
            self.__updateConfigurations(meta)
            self.renderer.render(result, ast, node)
            self.__callFunction(self.__renderer, 'postRender', node, args=(result, node, ast))
            self.__executeExtensionFunction('postRender', node,
                                            args=(result, node, meta, self.__renderer))
            self.__resetConfigurations()

            self.__renderer.write(node, result.root)

    def __finalize(self, other_nodes, num_threads):
        """
        Complete copying of non-source (e.g., images) files as well as perform indexing.
        """
        start = time.time()
        for node in other_nodes:
            self.renderer.write(node)
        return time.time() - start

    def __callFunction(self, obj, name, page, args=tuple()):
        """Helper for calling a function on the supplied object."""
        func = getattr(obj, name, None)
        if func is not None:
            try:
                func(*args)
            except Exception:
                msg = "Failed to execute '%s' method within the '%s' object.\n"
                if page is not None:
                    msg += "  Error occurred in {}\n".format(page.local)
                msg += mooseutils.colorText(traceback.format_exc(), 'GREY')
                LOG.critical(msg, name, obj.__class__.__name__)

    def __executeExtensionFunction(self, name, page, args=tuple()):
        """Execute pre/post functions for extensions, reader, and renderer."""

        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('name', name, str)

        for ext in self.__extensions:
            if ext.active:
                self.__callFunction(ext, name, page, args)

    def __updateConfigurations(self, meta):
        """Update configuration from meta data."""
        self.__reader.update(error_on_unknown=False, **meta.getData('reader'))
        self.__renderer.update(error_on_unknown=False, **meta.getData('renderer'))
        for ext in self.__extensions:
            ext.update(error_on_unknown=False, **meta.getData(ext.__class__.__name__))

    def __resetConfigurations(self):
        """Reset configuration to original state."""
        self.__reader.resetConfig()
        self.__renderer.resetConfig()
        for ext in self.__extensions:
            ext.resetConfig()

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
