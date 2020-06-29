#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
"""Module for defining Executioner objects, which are helpers for tokenization and rendering."""
import sys
import os
import copy
import time
import logging
import traceback
import multiprocessing
import mooseutils
import random

import MooseDocs
from ..tree import pages
from ..common import exceptions, mixins

LOG = logging.getLogger('MooseDocs.Executioner')

class Executioner(mixins.ConfigObject, mixins.TranslatorObject):
    """
    Base object for tokenization and rendering computations.

    The Translator object is responsible for managing the conversion from the source files
    to the final result via tokenization and rendering. However, how this process executes
    can vary, especially when the multiprocessing is considered so to allow for different methods
    this class was created.

    This is an internal object that should not be used directly, the Translator object should
    provide the necessary interface for all Extension operations. If access to this object is needed
    then the Translator is not designed correctly.

    Significant effort has been made to correctly handle the data involved with translation
    using the multiprocessing package. Throughout the development of MooseDocs the
    algorithm used has changed in an attempt to optimize performance. There has not been one
    method that has been ideal for all cases, so rather than be restricted to a single algorithm
    the Executioner system was created to allow for the algorithm to be changed as well as
    new algorithms be developed.

    In general, the conversion process is as follows. How this is achieved varies in each
    Executioner

        1. Execute preExecute methods
        2. Reading raw content (following called for each Page object)
           2.1. Execute preRead methods
           2.2. Read content from page.Source files, using Translator.read method
           2.3. Execute postRead methods
        3. Tokenize pages.Source pages (following called for each Page object)
           3.1. Execute preTokenize methods
           3.2. Perform tokenization, using Translator.tokenize method
           3.3. Execute postTokenize methods
        3. Render pages.Source pages (following called for each Page object)
           3.1. Execute preRender methods
           3.2. Perform rendering, using Translator.tokenize method
           3.3. Execute postRender methods
        4. Write pages.Source pages (following called for each Page object)
           3.1. Execute preWrite methods
           3.2. Perform write, using Translator.write method
           3.3. Execute postWrite methods
        5. Finalize the content, i.e., copy pages.File objects to the destination
        6. Execute postExecute methods

    The following are the total and translation times for the various Executioners for the
    complete MOOSE website in modules/doc using 12 cores on a Mac trash can ("Mac Pro (Late 2013)"):

                     Total / Translate (sec.)
             Serial:   819 / 792
    ParallelBarrier:   233 / 210
      ParallelQueue:   187 / 163
       ParallelPipe:   296 / 269
    """
    #: A multiprocessing lock. This is used in various locations, mainly prior to caching items
    #  as well as during directory creation.
    LOCK = multiprocessing.Lock()

    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['profile'] = (False, "Run the executioner with profiling.")
        return config

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorObject.__init__(self)
        self._page_objects = list()

    def init(self, destination):
        """Initialize the Page objects."""

        # Call Extension init() method
        LOG.info('Executing extension init() methods...')
        t = time.time()
        self.translator.executeMethod('init')
        LOG.info('Executing extension init() methods complete [%s sec.]', time.time() - t)

        # Initialize Page objects
        LOG.info('Executing extension initPage() methods...')
        t = time.time()
        for node in self._page_objects:

            # Setup destination and output extension
            node.base = destination
            if isinstance(node, pages.Source):
                node.output_extension = self.translator.renderer.EXTENSION

            # Setup Page Config
            config = dict()
            for ext in self.translator.extensions:
                node['__{}__'.format(ext.name)] = dict()
            self.translator.executePageMethod('initPage', node)
            Executioner.setMutable(node, False)

        LOG.info('Executing extension initPage() methods complete [%s sec.]', time.time() - t)

    def addPage(self, page):
        """Add a Page object to be Translated."""
        page._Page__unique_id = len(self._page_objects)
        self._page_objects.append(page)

    def getPages(self):
        """Return a list of Page objects."""
        return self._page_objects

    def execute(self, nodes, num_threads=1):
        """
        Perform the translation.

        Inputs:
            nodes[list]: A list of page objects to convert.
            num_threads[int]: The number of threads to use for execution.
        """
        raise NotImplementedError("The execute method must be defined.")

    @staticmethod
    def setMutable(node, value):
        """Helper for controlling the mutable state of Page objects.

        When MooseDocs runs in parallel the Page objects are copied to sub processes using the
        multiprocessing module, thus during operation calculations are preformed using the copies.
        If the attributes are altered, only the copies are altered not the original from the
        root process.

        However, on the process responsible for performing the calculation the copy of the Page
        object attributes is returned and used to update the page on the root process. The MooseDocs
        system allows for a Page object to be retrieved from anywhere. This flag prevents
        modification of attributes from processes that do not own the object to avoid misleading
        results.
        """
        node._AutoPropertyMixin__mutable = value

    def __call__(self, nodes, num_threads=1):
        """
        Called by Translator object, this executes the steps listed in the class description.
        """
        total = time.time()
        self.assertInitialized()
        self.translator.executeMethod('preExecute', log=True)

        nodes = nodes or self.getPages()
        source_nodes = [n for n in nodes if isinstance(n, pages.Source)]
        other_nodes = [n for n in nodes if not isinstance(n, pages.Source)]

        if source_nodes:
            t = time.time()
            LOG.info('Translating using %s threads...', num_threads)
            n = len(source_nodes) if num_threads > len(source_nodes) else num_threads
            self.execute(source_nodes, n)
            LOG.info('Translating complete [%s sec.]', time.time() - t)

        # Indexing/copying
        if other_nodes:
            LOG.info('Copying content...')
            n = len(other_nodes) if num_threads > len(other_nodes) else num_threads
            t = self.finalize(other_nodes, n)
            LOG.info('Copying Finished [%s sec.]', t)

        self.translator.executeMethod('postExecute', log=True)

        LOG.info('Total Time [%s sec.]', time.time() - total)

    def read(self, node):
        """Perform reading of page content."""
        content = None
        if node.get('active', True):
            self.translator.executePageMethod('preRead', node)
            content = self.translator.reader.read(node) if node.get('read', True) else ''
            self.translator.executePageMethod('postRead', node, args=(copy.copy(content),))

        return content

    def tokenize(self, node, content):
        """Perform tokenization and call all associated callbacks for the supplied page."""

        ast = self.translator.reader.getRoot()
        if node.get('active', True):
            self.translator.executePageMethod('preTokenize', node, args=(ast,))
            if node.get('tokenize', True):
                self.translator.reader.tokenize(ast, content, node)
            self.translator.executePageMethod('postTokenize', node, args=(ast,))

        return ast

    def render(self, node, ast):
        """Perform rendering and call all associated callbacks for the supplied page."""

        result = self.translator.renderer.getRoot()
        if node.get('active', True):
            self.translator.executePageMethod('preRender', node, args=(result,))
            if node.get('render', True):
                self.translator.renderer.render(result, ast, node)
            self.translator.executePageMethod('postRender', node, args=(result,))

        return result

    def write(self, node, result):
        """Perform writing and call all associated callbacks for the supplied page."""

        if node.get('active', True):
            self.translator.executePageMethod('preWrite', node, args=(result,))
            if node.get('write', True):
                self.translator.renderer.write(node, result.root)
            self.translator.executePageMethod('postWrite', node)
        return result

    def finalize(self, other_nodes, num_threads):
        """Complete copying of non-source (e.g., images) files."""
        start = time.time()
        for node in other_nodes:
            self.translator.renderer.write(node)
        return time.time() - start

class Serial(Executioner):
    """Simple serial Executioner, this is useful for debugging."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._page_content = dict()
        self._page_ast = dict()
        self._page_result = dict()

    def execute(self, nodes, num_threads=1):
        """Perform the translation, in serial."""
        self._run(nodes, self._readHelper, 'Read')
        self._run(nodes, self._tokenizeHelper, 'Tokenize')
        self._run(nodes, self._renderHelper, 'Render')
        self._run(nodes, self._writeHelper, 'Write')

    def _run(self, nodes, target, prefix):
        """Run and optionally profile a function"""

        t = time.time()
        LOG.info('%s...', prefix)

        if self.get('profile', False):
            mooseutils.run_profile(target, nodes)
        else:
            target(nodes)

        LOG.info('Finished %s [%s sec.]', prefix, time.time() - t)

    def _readHelper(self, nodes):
        for node in nodes:
            Executioner.setMutable(node, True)
            content = self.read(node)
            self._page_content[node.uid] = content

    def _tokenizeHelper(self, nodes):
        for node in nodes:
            ast = self.tokenize(node, self._page_content[node.uid])
            self._page_ast[node.uid] = ast

    def _renderHelper(self, nodes):
        for node in nodes:
            result = self.render(node, self._page_ast[node.uid])
            self._page_result[node.uid] = result

    def _writeHelper(self, nodes):
        for node in nodes:
            self.write(node, self._page_result[node.uid])
            Executioner.setMutable(node, False)

class ParallelQueue(Executioner):
    """
    Implements a Queue based execution for each step of the translation.

    Follows Queue example:
    https://docs.python.org/3/library/multiprocessing.html#multiprocessing-examples
    """
    STOP = float('inf')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._page_content = None
        self._page_ast = None
        self._page_result = None

    def execute(self, nodes, num_threads=1):

        n = len(self.getPages())
        self._page_content = [None]*n
        self._page_ast = [None]*n
        self._page_result = [None]*n

        # READ
        self._run(nodes, self._page_content, self._read_target, num_threads,
                  lambda n: -os.path.getsize(n.source) if os.path.isfile(n.source) else float('-inf'),
                  "Reading")

        # TOKENIZE
        self._run(nodes, self._page_ast, self._tokenize_target, num_threads,
                  lambda n: -len(self._page_content[n.uid]),
                  "Tokenizing")

        # RENDER
        self._run(nodes, self._page_result, self._render_target, num_threads,
                  lambda n: -self._page_ast[n.uid].count,
                  "Rendering")

        # WRITE
        self._run(nodes, None, self._write_target, num_threads,
                  lambda n: -self._page_result[n.uid].count,
                  "Writing")

    def _run(self, nodes, container, target, num_threads=1, key=None, prefix='Running'):
        """Helper function for running in parallel using Queues"""

        # Time the process
        t = time.time()
        LOG.info('%s using %s threads...', prefix, num_threads)

        # Input/Output Queue object
        page_queue = multiprocessing.Queue()
        output_queue = multiprocessing.Queue()

        # Populate the input Queue with the ids for the nodes to be read
        random.shuffle(nodes)
        for node in nodes:
            page_queue.put(node.uid)

        # Create Processes for each thread that reads the data and updates the Queue objects
        for i in range(num_threads):
            multiprocessing.Process(target=target, args=(page_queue, output_queue)).start()

        # Extract the data from the output Queue object and update Page object attributes
        for i in range(len(nodes)):
            uid, attributes, out = output_queue.get()
            if container is not None:
                container[uid] = out
            node = self._page_objects[uid]
            Executioner.setMutable(node, True)
            node.attributes.update(attributes)
            Executioner.setMutable(node, False)

        for i in range(num_threads):
            page_queue.put(ParallelQueue.STOP)

        LOG.info('Finished %s [%s sec.]', prefix, time.time() - t)

    def _read_target(self, qin, qout):
        """Function for calling self.read with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._page_objects[uid]
            Executioner.setMutable(node, True)
            content = self.read(node)
            qout.put((uid, node.attributes, content))
            Executioner.setMutable(node, False)

    def _tokenize_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._page_objects[uid]
            content = self._page_content[uid]
            Executioner.setMutable(node, True)
            ast = self.tokenize(node, content)
            qout.put((uid, node.attributes, ast))
            Executioner.setMutable(node, False)

    def _render_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._page_objects[uid]
            ast = self._page_ast[uid]
            Executioner.setMutable(node, True)
            result = self.render(node, ast)
            qout.put((uid, node.attributes, result))
            Executioner.setMutable(node, False)

    def _write_target(self, qin, qout):
        """Function for calling self.write with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._page_objects[uid]
            result = self._page_result[uid]
            Executioner.setMutable(node, True)
            ast = self.write(node, result)
            qout.put((uid, node.attributes, None))
            Executioner.setMutable(node, False)

class ParallelBarrier(Executioner):
    """
    Parallel Executioner that uses shared multiprocessing.Manager dictionaries
    for storing the Page data across processors and a parallel barrier between the
    tokenization and rendering steps to be sure that the data is completed
    for all pages prior to beginning the various steps.
    """
    def execute(self, nodes, num_threads=1):
        """Perform the translation with multiprocessing."""

        barrier = multiprocessing.Barrier(num_threads)
        manager = multiprocessing.Manager()
        page_attributes = manager.list([None]*len(self._page_objects))

        # Initialize the page attributes container using the existing list of Page node objects
        for i in range(len(page_attributes)):
            Executioner.setMutable(self._page_objects[i], True)
            page_attributes[i] = self._page_objects[i].attributes
            Executioner.setMutable(self._page_objects[i], False)

        jobs = []
        random.shuffle(nodes)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = multiprocessing.Process(target=self._target, args=(chunk, barrier, page_attributes))
            jobs.append(p)
            p.start()

        for job in jobs:
            job.join()

        # This is needed to maintain the page attributes during live serving. In parallel, when the
        # Executioner executes each process created above gets a copy of self._page_objects. Each
        # process is running the _target method and keeping the attributes of the pages up to date
        # across the processes. This call updates the attributes of the original pages that
        # were copied when the processes start. Thus, when new processes are started during a
        # live reload the attributes are correct when the copy is performed again for the new
        # processes.
        self._updateAttributes(page_attributes)

    def _target(self, nodes, barrier, page_attributes):
        """Target function for multiprocessing.Process calls."""

        local_content = dict()
        local_ast = dict()
        local_result = dict()

        # READ
        for node in nodes:
            Executioner.setMutable(node, True)
            content = self.read(node)
            page_attributes[node.uid] = node.attributes
            Executioner.setMutable(node, False)
            local_content[node.uid] = content

        barrier.wait()
        self._updateAttributes(page_attributes)

        # TOKENIZE
        for node in nodes:
            content = local_content.pop(node.uid)
            Executioner.setMutable(node, True)
            mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
            ast = self.tokenize(node, content)
            page_attributes[node.uid] = node.attributes
            Executioner.setMutable(node, False)
            local_ast[node.uid] = ast

        barrier.wait()
        self._updateAttributes(page_attributes)

        # RENDER
        for node in nodes:
            ast = local_ast.pop(node.uid)
            Executioner.setMutable(node, True)
            mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
            result = self.render(node, ast)
            page_attributes[node.uid] = node.attributes
            Executioner.setMutable(node, False)
            local_result[node.uid] = result

        barrier.wait()
        self._updateAttributes(page_attributes)

        # WRITE
        for node in nodes:
            result = local_result.pop(node.uid)
            Executioner.setMutable(node, True)
            mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
            result = self.write(node, result)
            Executioner.setMutable(node, False)

    def _updateAttributes(self, page_attributes):
        """Update the local page objects with attributes gathered from the other processes"""
        for i, page in enumerate(self._page_objects):
            if page_attributes[i] is not None:
                page.update(page_attributes[i])

class ParallelPipe(Executioner):
    """
    Parallel execution that performs operations and transfers data using multiprocessing.Pipe
    """
    STOP = float('inf')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self._page_content = None
        self._page_ast = None
        self._page_result = None

    def execute(self, nodes, num_threads=1):

        n = len(self.getPages())
        self._page_content = [None]*n
        self._page_ast = [None]*n
        self._page_result = [None]*n

        # READ
        self._run(nodes, self._page_content, self._read_target, num_threads, "Reading")

        # TOKENIZE
        self._run(nodes, self._page_ast, self._tokenize_target, num_threads, "Tokenizing")

        # RENDER
        self._run(nodes, self._page_result, self._render_target, num_threads, "Rendering")

        # WRITE
        self._run(nodes, None, self._write_target, num_threads, "Writing")

    def _run(self, nodes, container, target, num_threads=1, prefix='Running'):
        """Helper function for running in parallel using Pipe"""

        # Time the process
        t = time.time()
        LOG.info('%s using %s threads...', prefix, num_threads)

        # Tokenization
        jobs = []
        conn1, conn2 = multiprocessing.Pipe(False)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = multiprocessing.Process(target=target, args=(chunk, conn2))
            p.start()
            jobs.append(p)

        while any(job.is_alive() for job in jobs):
            if conn1.poll():
                data = conn1.recv()
                for uid, attributes, out in data:
                    node = self._page_objects[uid]
                    Executioner.setMutable(node, True)
                    node.attributes.update(attributes)
                    Executioner.setMutable(node, False)

                    if container is not None:
                        container[uid] = out

        LOG.info('Finished %s [%s sec.]', prefix, time.time() - t)

    def _read_target(self, nodes, conn):
        """Function for calling self.read with Connection object"""

        data = list()
        for node in nodes:
            Executioner.setMutable(node, True)
            content = self.read(node)
            data.append((node.uid, node.attributes, content))
            Executioner.setMutable(node, False)

        with self.LOCK:
            conn.send(data)

    def _tokenize_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            content = self._page_content[node.uid]
            Executioner.setMutable(node, True)
            ast = self.tokenize(node, content)
            data.append((node.uid, node.attributes, ast))
            Executioner.setMutable(node, False)

        with self.LOCK:
            conn.send(data)

    def _render_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            ast = self._page_ast[node.uid]
            Executioner.setMutable(node, True)
            result = self.render(node, ast)
            data.append((node.uid, node.attributes, result))
            Executioner.setMutable(node, False)

        with self.LOCK:
            conn.send(data)

    def _write_target(self, nodes, conn):
        """Function for calling self.write with Connection object"""

        data = list()
        for node in nodes:
            result = self._page_result[node.uid]
            Executioner.setMutable(node, True)
            self.write(node, result)
            data.append((node.uid, node.attributes, None))
            Executioner.setMutable(node, False)

        with self.LOCK:
            conn.send(data)
