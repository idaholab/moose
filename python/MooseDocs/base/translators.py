"""
Module that defines Translator objects for converted AST from Reader to Rendered output from
Renderer objects. The Translator objects exist as a place to import extensions and bridge
between the reading and rendering content.
"""
import os
import logging
import multiprocessing
import time
import json

import anytree

import mooseutils

import MooseDocs
from MooseDocs import common
from MooseDocs.common import mixins, exceptions
from MooseDocs.tree import page
from components import Extension
from readers import Reader
from renderers import Renderer, MaterializeRenderer

LOG = logging.getLogger('MooseDocs.Translator')

class Translator(mixins.ConfigObject):
    """
    Object responsible for converting reader content into an AST and rendering with the
    supplied renderer.

    TODO: It would be a better design to separate the translator from the components, rather than
          the components having access to the Translator object perhaps the TokenizeComponent should
          have access to only the Reader and RenderComponent only have the Renderer. This would
          avoid the temptation from tokenizing in the renderer and rendering in the tokenizer. To do
          this the parallel implementation needs to be improved, which is not all the easy in
          python. For a discussion of why, see the "execute" method below.

    Inputs:
        reader: [Reader] A Reader instance.
        renderer: [Renderer] A Renderer instance.
        extensions: [list] A list of extensions objects to use.
    """
    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['destination'] = (os.path.join(os.getenv('HOME'), '.local', 'share', 'moose',
                                              'site'),
                                 "The output directory.")
        return config

    def __init__(self, content, reader, renderer, extensions, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)

        common.check_type('content', content, page.PageNodeBase)
        common.check_type('reader', reader, Reader)
        common.check_type('renderer', renderer, Renderer)
        common.check_type('extensions', extensions, list)
        for ext in extensions:
            common.check_type('extensions', ext, Extension)

        self.__initialized = False
        self.__current = None
        self.__lock = multiprocessing.Lock()
        self.__root = content
        self.__extensions = extensions
        self.__reader = reader
        self.__renderer = renderer
        self.__destination = None # assigned during init()
        self.__extension_functions = dict(preRender=list(),
                                          postRender=list(),
                                          preTokenize=list(),
                                          postTokenize=list())

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
    def root(self):
        """Return the root page of the documents being translated."""
        return self.__root

    @property
    def current(self):
        """Return the current page being converted, see build method."""
        return self.__current

    @current.setter
    def current(self, value):
        """Set the current page being translated, see page.MarkdownNode.build()."""
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('value', value, (type(None), page.PageNodeBase))
        self.__current = value

    @property
    def lock(self):
        """Return a multiprocessing lock for serial operations (e.g., directory creation)."""
        return self.__lock

    def update(self, **kwargs):
        """Update configuration and handle destination."""
        dest = kwargs.get('destination', None)
        if dest is not None:
            kwargs['destination'] = mooseutils.eval_path(dest)
        mixins.ConfigObject.update(self, **kwargs)

    def init(self):
        """
        Initialize the translator with the output destination for the converted content.

        This method also initializes all the various items within the translator for performing
        the conversion.

        Inputs:
            destination[str]: The path to the output directory.
        """
        if self.__initialized:
            msg = "The {} object has already been initialized, this method should not " \
                  "be called twice."
            raise MooseDocs.common.exceptions.MooseDocsException(msg, type(self))

        destination = self.get("destination")
        self.__reader.init(self)
        self.__renderer.init(self)

        for ext in self.__extensions:
            common.check_type('extensions', ext, MooseDocs.base.components.Extension)
            ext.init(self)
            ext.extend(self.__reader, self.__renderer)
            for comp in self.__reader.components:
                if comp.extension is None:
                    comp.extension = ext
            for comp in self.__renderer.components:
                if comp.extension is None:
                    comp.extension = ext

            for func_name in self.__extension_functions:
                if hasattr(ext, func_name):
                    self.__extension_functions[func_name].append(getattr(ext, func_name))

        for node in anytree.PreOrderIter(self.__root):
            node.base = destination
            node.init(self)

        self.__initialized = True

    def executeExtensionFunction(self, name, *args):
        """
        Execute pre/post functions for extensions.
        """
        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            common.check_type('name', name, str)
            if name not in self.__extension_functions:
                msg = "The supplied extension function name '{}' does not exist, the possible " \
                      "names include: {}."
                raise exceptions.MooseDocsException(msg, name, self.__extension_functions.keys())

        for func in self.__extension_functions[name]:
            func(*args)

    def reinit(self):
        """
        Reinitialize the Reader, Renderer, and all Extension objects.
        """
        self.__assertInitialize()
        self.reader.reinit()
        self.renderer.reinit()

        for ext in self.__extensions:
            ext.reinit()

    def execute(self, num_threads=1):
        """
        Perform parallel build for all pages.

        Inputs:
            num_threads[int]: The number of threads to use (default: 1).

        NOTICE:
        A proper parallelization for MooseDocs would be three parallel steps, with minimal
        communication.
          1. Read all the markdown files (in parallel).
          2. Perform the AST tokenization (in parallel), then communicate the completed
             AST back to the main process.
          3. Convert the AST to HTML (in parallel).
          4. Write/copy (in parallel) the completed HTML and other files (images, js, etc.).

        However, step two is problematic because python requires that the AST be pickled,
        which is possible, for communication. In doing this I realized that the pickling was a
        limiting factor and made the AST step very slow. I need to investigate this further to
        make sure I was using a non-locking pool of workers, but this was taking too much
        development time.

        The current implementation performs all four steps together, which generally works just
        fine, with one exception. The autolink extension actually interrogates the AST from other
        pages. Hence, if the other page was generated off process the information is not available.
        The current implementation will just compute the AST locally (i.e., I am performing repeated
        calculations in favor of communication). This works well enough for now, but as more
        autolinking is preformed and other similar extensions are created this could cause a slow
        down.

        Long term this should be looked into again, for now the current approach is working well.
        This new system is already an order of 4 times faster than the previous implementation and
        likely could be optimized further.

        The multiprocessing.Manager() needs to be explored, it is working to pull the JSON index
        information together.
        """
        common.check_type('num_threads', num_threads, int)
        self.__assertInitialize()

        self.renderer.preExecute()


        # Log start message and time
        LOG.info("Building Pages...")
        start = time.time()

        manager = multiprocessing.Manager()
        array = manager.list()
        build_index = isinstance(self.renderer, MaterializeRenderer)
        def target(nodes, lock):
            """Helper for building multiple nodes (i.e., a chunk for a process)."""
            for node in nodes:
                node.build()
                if isinstance(node, page.MarkdownNode):
                    if build_index:
                        node.buildIndex(self.renderer.get('home', None))
                        with lock:
                            for entry in node.index:
                                array.append(entry)

        # Complete list of nodes
        nodes = [n for n in anytree.PreOrderIter(self.root)]

        # Serial
        if num_threads == 1:
            target(nodes, self.lock)

        # Multiprocessing
        else:
            jobs = []
            for chunk in mooseutils.make_chunks(nodes, num_threads):
                p = multiprocessing.Process(target=target, args=(chunk, self.lock))
                p.start()
                jobs.append(p)

            for job in jobs:
                job.join()

        # Done
        stop = time.time()
        LOG.info("Build time %s sec.", stop - start)

        if build_index:
            iname = os.path.join(self.get('destination'), 'js', 'search_index.js')
            if not os.path.isdir(os.path.dirname(iname)):
                os.makedirs(os.path.dirname(iname))
            items = [v for v in array if v]
            common.write(iname, 'var index_data = {};'.format(json.dumps(items)))

        self.renderer.postExecute()

    def __assertInitialize(self):
        """Helper for asserting initialize status."""
        if not self.__initialized:
            msg = "The Translator.init() method must be called prior to executing this method."
            raise exceptions.MooseDocsException(msg)
