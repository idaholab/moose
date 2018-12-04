#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
"""Module for defining Executioner objects, which are helpers for tokenization and rendering."""
import time
import logging
import traceback
import multiprocessing
import mooseutils
import MooseDocs
from MooseDocs.tree import pages
from MooseDocs.common import exceptions, mixins, check_type

LOG = logging.getLogger('MooseDocs.Executioner')

class Meta(object):
    """
    Config data object for data on the pages.Page objects.

    The primary purpose for this object is to enable the ability to modify configurations of the
    reader, renderer, and extension objects from within an extension (i.e., the config extension).
    """
    def __init__(self):
        self.__data = dict()

    def initData(self, key, default=None):
        """Initialize dict key with the supplied type."""
        self.__data[key] = default

    def getData(self, key):
        """Retrieve data for the supplied key."""
        return self.__data[key]

    def setData(self, key, value):
        """Set the data for the supplied key."""
        self.__data[key] = value

class Executioner(mixins.ConfigObject, mixins.TranslatorMixin):
    """
    Base object for tokenization and rendering computations.

    The Translator object is responsible for managing the conversion from the source files
    to the final result via tokenization and rendering. However, how this process executes
    can vary, especially when the multiprocessing is considered.

    Significant effort has been made to correctly handle the data involved with translation
    using the multiprocessing package. Throughout the development of MooseDocs the
    algorithm used has changed in an attempt to optimize performance. There has not been one
    method that has been ideal for all cases, so rather than be restricted to a single algorithm
    the Executioner system was created to allow for the algorithm to be changed as well as
    new algorithms be developed.

    In general, the conversion process is as follows. How this is achieved varies in each
    Executioner

        1. Execute preExecute methods
        2. Convert pages.Source pages (following called for each Page object)
           2.1. Read content from page.Source files
           2.2. Execute postRead methods
           2.3. Create the root AST object via Reader::getRoot()
           2.4. Execute preTokenize methods
           2.5. Perform tokenization
           2.6. Execute postTokenize methods
        3. Convert AST into rendered results  (following called for each Page object)
           3.1. Create the root result object via Renderer::getRoot()
           3.2. Execute preRender methods
           3.3. Render the AST
           3.4. Execute postRender methods
           3.5. Write the results
           3.6. Call postWrite methods
        4. Finalize the content, i.e., copy pages.File objects to the destination
        5. Execute postExecute methods

    Inputs:
        translator[base.Translator]: The translator object to be used for conversion.
    """
    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['profile'] = (False, "Run the executioner with profiling.")
        return config

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorMixin.__init__(self)
        self._meta_data = dict()
        self._tree_data = dict()
        self._ast_available = False

    def isSyntaxTreeAvailable(self):
        """Returns True if the AST creation is complete."""
        return self._ast_available

    def getSyntaxTree(self, page):
        """Return a copy of the syntax tree for the supplied page."""

        if not self.isSyntaxTreeAvailable():
            msg = "The AST data for {} page is not available until tokenization is complete, " \
                  "therefore this method should not be used within the pre/postTokenize or " \
                  "createToken methods."
            raise exceptions.MooseDocsException(msg, page.local)

        elif not isinstance(page, pages.Source):
            msg = "AST data is only available for pages.Source objects; however, a {} object " \
                  "was provided."
            raise exceptions.MooseDocsException(msg, page.__class__.__name__)

        data = self._tree_data.get(page.uid, None)
        if data is not None:
            data = data.copy()

        return data

    def getMetaData(self, page, key):
        """Return the desired meta data for the supplied page."""

        if not self.isSyntaxTreeAvailable():
            msg = "The meta data for {} page is not available until tokenization is complete, " \
                  "therefore this method should not be used within the pre/postTokenize or " \
                  "createToken methods."
            raise exceptions.MooseDocsException(msg, page.local)

        meta = self._meta_data.get(page.uid, None)
        if meta is not None:
            return meta.getData(key)

    def execute(self, nodes, num_threads=1):
        """
        Perform the translation.

        Inputs:
            nodes[list]: A list of page objects to convert.
            num_threads[int]: The number of threads to use for execution.
        """
        raise NotImplementedError("The execute method must be defined.")

    def __call__(self, nodes, num_threads=1):
        """
        Called by Translator object, this executes the steps listed in the class description.
        """
        total = time.time()
        self.assertInitialized()

        nodes = nodes or self.translator.content
        source_nodes = [n for n in nodes if isinstance(n, pages.Source)]
        other_nodes = [n for n in nodes if not isinstance(n, pages.Source)]

        LOG.info('Executing preExecute methods...')
        t = time.time()
        self._executeExtensionFunction('preExecute', None, args=(self.translator.content,))
        LOG.info('Finished preExecute methods [%s sec.]', time.time() - t)

        if source_nodes:
            t = time.time()
            LOG.info('Translating using %s threads...', num_threads)
            if self.get('profile', False):
                mooseutils.run_profile(self.execute, source_nodes, num_threads)
            else:
                self.execute(source_nodes, num_threads)
            LOG.info('Translating complete [%s sec.]', time.time() - t)

        # Indexing/copying
        if other_nodes:
            LOG.info('Copying content...')
            t = self.finalize(other_nodes, num_threads)
            LOG.info('Copying Finished [%s sec.]', t)

        LOG.info('Executing postExecute methods...')
        t = time.time()
        self._executeExtensionFunction('postExecute', None, args=(self.translator.content,))
        LOG.info('Finished postExecute methods [%s sec.]', time.time() - t)

        LOG.info('Total Time [%s sec.]', time.time() - total)

    def tokenize(self, node):
        """Perform tokenization and call all associated callbaccks for the supplied page."""

        meta = Meta()
        self._executeExtensionFunction('initMetaData', node, args=(node, meta))
        content = self.translator.reader.read(node)
        self._executeExtensionFunction('postRead', node, args=(content, node, meta))

        ast = self.translator.reader.getRoot()
        self._callFunction(self.translator.reader, 'preTokenize', node, args=(ast, node, meta))
        self._executeExtensionFunction('preTokenize', node,
                                       args=(ast, node, meta, self.translator.reader))

        self.translator.reader.tokenize(ast, content, node)

        self._callFunction(self.translator.reader, 'postTokenize', node, args=(ast, node, meta))
        self._executeExtensionFunction('postTokenize', node,
                                       args=(ast, node, meta, self.translator.reader))

        #self.translator.resetConfigurations()
        return ast, meta

    def render(self, node, ast, meta):
        """Perform rendering and call all associated callbaccks for the supplied page."""

        result = self.translator.renderer.getRoot()
        self._callFunction(self.translator.renderer, 'preRender', node, args=(result, node, meta))
        self._executeExtensionFunction('preRender', node,
                                       args=(result, node, meta, self.translator.renderer))

        self.translator.renderer.render(result, ast, node)

        self._callFunction(self.translator.renderer, 'postRender', node, args=(result, node, ast))
        self._executeExtensionFunction('postRender', node,
                                       args=(result, node, meta, self.translator.renderer))

        self.translator.renderer.write(node, result.root)
        self._executeExtensionFunction('postWrite', node)

    def finalize(self, other_nodes, num_threads):
        """Complete copying of non-source (e.g., images) files."""
        start = time.time()
        for node in other_nodes:
            self.translator.renderer.write(node)
        return time.time() - start

    def _executeExtensionFunction(self, name, page, args=tuple()):
        """Helper to call pre/post functions for extensions, reader, and renderer."""

        if MooseDocs.LOG_LEVEL == logging.DEBUG:
            check_type('name', name, str)

        for ext in self.translator.extensions:
            if ext.active:
                self._callFunction(ext, name, page, args)

    @staticmethod
    def _callFunction(obj, name, page, args=tuple()):
        """Helper for calling a function on the supplied object."""
        func = getattr(obj, name, None)
        if func is not None:
            try:
                func(*args)
            except Exception:
                msg = "Failed to execute '{}' method within the '{}' object.\n" \
                      .format(name, obj.__class__.__name__)
                if page is not None:
                    msg += "  Error occurred in {}\n".format(page.local)
                msg += mooseutils.colorText(traceback.format_exc(), 'GREY')
                LOG.critical(msg)#, name, obj.__class__.__name__)

class Serial(Executioner):
    """Simple serial Executioner, this is useful for debugging."""
    def __init__(self, *args, **kwargs):
        super(Serial, self).__init__(*args, **kwargs)

    def execute(self, nodes, num_threads=1):
        """Perform the translation, in serial."""
        self.__tokenize_helper(nodes)
        self._ast_available = True
        self.__render_helper(nodes)

    def __tokenize_helper(self, nodes):
        """Helper for tokenization."""
        for node in nodes:
            ast, meta = self.tokenize(node)
            self._tree_data[node.uid] = ast
            self._meta_data[node.uid] = meta

    def __render_helper(self, nodes):
        """Helper for rendereing."""
        for node in nodes:
            ast = self._tree_data[node.uid]
            meta = self._meta_data[node.uid]
            self.render(node, ast, meta)


class ParallelBarrier(Executioner):
    """
    Parallel Executioner that uses shared multiprocessing.Manager dictionaries
    for storing the data across processors and a parallel barrier between the
    tokenization and rendering steps to be sure that the AST data is completed
    for all pages prior to beginning rendering.

    The Manager dict() are "shared" across processes, so in essence this class
    performs tokenzation and broadcasts the AST and meta data to all other processes,
    so when tokenization is completed all processors have the data.
    """

    def __init__(self, *args, **kwargs):
        super(ParallelBarrier, self).__init__(*args, **kwargs)
        self._manager = multiprocessing.Manager()
        self._meta_data = self._manager.dict()
        self._tree_data = self._manager.dict()
        self._ast_available = self._manager.Value('i', 0)

    def isSyntaxTreeAvailable(self):
        """Override to allow this check to work across processes."""
        return self._ast_available.value == 1

    def execute(self, nodes, num_threads=1):
        """Perform the translation with multiprocessing."""
        if num_threads > len(nodes):
            num_threads = len(nodes)

        barrier = mooseutils.parallel.Barrier(num_threads)

        jobs = []
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = multiprocessing.Process(target=self.__target, args=(chunk, barrier))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

    def __target(self, nodes, barrier):
        """Target function for multiprocessing.Process calls."""

        for node in nodes:
            ast, meta = self.tokenize(node)
            self._tree_data[node.uid] = ast
            self._meta_data[node.uid] = meta

        barrier.wait()
        self._ast_available.value = True

        for node in nodes:
            ast = self._tree_data[node.uid]
            meta = self._meta_data[node.uid]
            self.render(node, ast, meta)

class ParallelPipe(Executioner):
    """
    Parallel execution that performs tokenization and uses a multiprocessing.Pipe to
    send the data to the main process. I then starts a new parallel process for
    rendering, using the data populated from the first step.

    TODO: This isn't working correct, but the ParallelBarrier seams to be the
          fastest, so I will need to come back to this at some time.
    """
    PROCESS_FINISHED = -1

    def execute(self, nodes, num_threads=1):
        """Perform parallel conversion using multiprocessing Pipe."""

        if num_threads > len(nodes):
            num_threads = len(nodes)

        # Tokenization
        jobs = []
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            conn1, conn2 = multiprocessing.Pipe(False)
            p = multiprocessing.Process(target=self.__tokenize_target, args=(chunk, conn2))
            p.start()
            jobs.append((p, conn1, conn2))

        # Finish the jobs and collect data from the Pipe
        while any(job[0].is_alive() for job in jobs):
            for job, conn1, conn2 in jobs:
                if conn1.poll():
                    uid = conn1.recv()
                    if uid == ParallelPipe.PROCESS_FINISHED:
                        conn1.close()
                        job.join()
                        continue

                    self._tree_data[uid] = conn1.recv()
                    self._meta_data[uid] = conn1.recv()

        self._ast_available = True

        # Rendering
        jobs = []
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = multiprocessing.Process(target=self.__render_target, args=(chunk,))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()


    def __tokenize_target(self, nodes, conn):
        """Target for tokenization multiprocessing.Process calls."""
        for node in nodes:
            ast, meta = self.tokenize(node)
            conn.send(node.uid)
            conn.send(ast)
            conn.send(meta)

        conn.send(self.PROCESS_FINISHED)

    def __render_target(self, nodes):
        """Target for rendering multiprocessing.Process calls."""
        for node in nodes:
            ast = self._tree_data[node.uid]
            meta = self._meta_data[node.uid]
            self.render(node, ast, meta)

class ParallelDemand(Executioner):
    """
    Parallel execution that does not perform any communication and re-builds syntax trees
    within the getSyntaxTree method.


    NOTE: This doesn't work yet. The configuration changes get screwed up when the
          getSyntaxTree method is called from another page. I need to re-think the configuration
          handling to make this work.

          Fixing this is not a high priority, because the ParallelBarrier seems to be the fastest.
    """
    def __init__(self, *args, **kwargs):
        super(ParallelDemand, self).__init__(*args, **kwargs)
        self._ast_available = True

    def execute(self, nodes, num_threads=1):
        """Perform the translation with multiprocessing."""

        if num_threads > len(nodes):
            num_threads = len(nodes)

        jobs = []
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = multiprocessing.Process(target=self.__target, args=(chunk,))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

    def __target(self, nodes):
        """Target function for multiprocessing.Process calls."""

        for node in nodes:
            ast, meta = self.tokenize(node)
            self._tree_data[node.uid] = ast
            self._meta_data[node.uid] = meta
            self.render(node, ast, meta)

    def getSyntaxTree(self, page):
        """Return the syntax tree, if it doesn't exist build it first."""
        if isinstance(page, pages.Source) and (page.uid not in self._tree_data):
            self.__target([page])
        return super(ParallelDemand, self).getSyntaxTree(page)

    def getMetaData(self, page, key):
        """Return the meta data, if it doesn't exist build it first."""
        if isinstance(page, pages.Source) and (page.uid not in self._meta_data):
            self.__target([page])
        return super(ParallelDemand, self).getMetaData(page, key)
