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
import platform

import MooseDocs
from ..tree import pages
from ..common import mixins

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

    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['profile'] = (False, "Run the executioner with profiling.")
        return config

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorObject.__init__(self)
        self._page_objects = list()

        # The method for spawning processes changed to "spawn" for macOS in python 3.9. Currently,
        # MooseDocs does not work with this combination. This will be fixed in moosetools migration.
        if (platform.python_version() >= '3.9') and (platform.system() == 'Darwin'):
            self._ctx = multiprocessing.get_context('fork')
        else:
            self._ctx = multiprocessing

        # A lock used prior to caching items or during directory creation to prevent race conditions
        self._lock = self._ctx.Lock()

        # Containers for storing the ouputs from the read, tokenize, and render methods, as well as
        # another for storing miscellaneous data, respectively. For parallel executioners, these are
        # reset as multiprocessing ('self._ctx') Manager().dict() objects so that modifications are
        # properly handled and communicated across processors.
        self._page_content = dict()
        self._page_ast = dict()
        self._page_result = dict()
        self._global_attributes = dict()

    def init(self, nodes, destination):
        """Initialize the Page objects."""

        # Call Extension init() method
        LOG.info('Executing extension init() methods...')
        t = time.time()
        self.translator.executeMethod('init')
        LOG.info('Executing extension init() methods complete [%s sec.]', time.time() - t)

        # Initialize Page objects
        LOG.info('Executing extension initPage() methods...')
        t = time.time()
        for node in nodes:
            # Setup destination and output extension
            node.base = destination
            if isinstance(node, pages.Source):
                node.output_extension = self.translator.renderer.EXTENSION

            # Setup page attributes
            config = dict()
            for ext in self.translator.extensions:
                node.attributes['__{}__'.format(ext.name)] = dict()
            self.translator.executePageMethod('initPage', node)

        LOG.info('Executing extension initPage() methods complete [%s sec.]', time.time() - t)

    def addPage(self, page):
        """Add a Page object to be Translated."""
        self._page_objects.append(page)

    def getPages(self):
        """Return a list of Page objects."""
        return self._page_objects

    def setGlobalAttribute(self, key, value):
        """Set a global attribute to be communicated across processors."""
        with self._lock:
            mooseutils.recursive_update(self._global_attributes, {key: value})

    def getGlobalAttribute(self, key, default=None):
        """Get a global attribute that was communicated across processors."""
        return self._global_attributes.get(key, default)

    def getGlobalAttributeItems(self):
        """Return iterator to underlying global attribute dict."""
        return self._global_attributes.items()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        """
        Perform the translation.

        Inputs:
            nodes[list]: A list of page objects to convert.
            num_threads[int]: The number of threads to use for execution.
        """
        raise NotImplementedError("The execute method must be defined.")

    def __call__(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        """
        Called by Translator object, this executes the steps listed in the class description.
        """
        total = time.time()
        self.assertInitialized()

        if read:
            self.translator.executeMethod('preExecute', log=True)

        source_nodes = [n for n in nodes if isinstance(n, pages.Source)]
        if source_nodes:
            t = time.time()
            n = len(source_nodes) if num_threads > len(source_nodes) else num_threads
            LOG.info('Translating using %s threads...', n)
            self.execute(source_nodes, n, read, tokenize, render, write)
            LOG.info('Translating complete [%s sec.]', time.time() - t)

        # Indexing/copying
        if write:
            other_nodes = [n for n in nodes if not isinstance(n, pages.Source)]
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
        if node.attributes.get('active', True):
            self.translator.executePageMethod('preRead', node)
            content = self.translator.reader.read(node) if node.attributes.get('read', True) else ''
            self.translator.executePageMethod('postRead', node, args=(copy.copy(content),))

        return content

    def tokenize(self, node, content):
        """Perform tokenization and call all associated callbacks for the supplied page."""

        ast = self.translator.reader.getRoot()
        if node.attributes.get('active', True):
            self.translator.executePageMethod('preTokenize', node, args=(ast,))
            if node.attributes.get('tokenize', True):
                self.translator.reader.tokenize(ast, content, node)
            self.translator.executePageMethod('postTokenize', node, args=(ast,))

        return ast

    def render(self, node, ast):
        """Perform rendering and call all associated callbacks for the supplied page."""

        result = self.translator.renderer.getRoot()
        if node.attributes.get('active', True):
            self.translator.executePageMethod('preRender', node, args=(result,))
            if node.attributes.get('render', True):
                self.translator.renderer.render(result, ast, node)
            self.translator.executePageMethod('postRender', node, args=(result,))
        return result

    def write(self, node, result):
        """Perform writing and call all associated callbacks for the supplied page."""

        if node.attributes.get('active', True):
            self.translator.executePageMethod('preWrite', node, args=(result,))
            if node.attributes.get('write', True):
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
    """Simple serial Executioner, this is useful for debugging or for very small builds."""

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read:
            self._run(nodes, self._readHelper, 'Read')
        if tokenize:
            self._run(nodes, self._tokenizeHelper, 'Tokenize')
        if render:
            self._run(nodes, self._renderHelper, 'Render')
        if write:
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

class ParallelQueue(Executioner):
    """
    Implements a Queue based execution for each step of the translation.

    Follows Queue example:
    https://docs.python.org/3/library/multiprocessing.html#multiprocessing-examples
    """
    STOP = float('inf')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._global_attributes = self._ctx.Manager().dict()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read:
            if not self._page_content:
                self._page_content = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_content, self._read_target, num_threads,
                      lambda n: -os.path.getsize(n.source) if os.path.isfile(n.source) else float('-inf'),
                      "Reading")

        if tokenize:
            if not self._page_ast:
                self._page_ast = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_ast, self._tokenize_target, num_threads,
                      lambda n: -len(self._page_content[n.uid]),
                      "Tokenizing")

        if render:
            if not self._page_result:
                self._page_result = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_result, self._render_target, num_threads,
                      lambda n: -self._page_ast[n.uid].count,
                      "Rendering")

        if write:
            self._run(nodes, None, self._write_target, num_threads,
                      lambda n: -self._page_result[n.uid].count,
                      "Writing")

    def _run(self, nodes, container, target, num_threads=1, key=None, prefix='Running'):
        """Helper function for running in parallel using Queues"""

        # Time the process
        t = time.time()
        LOG.info('%s using %s threads...', prefix, num_threads)

        # Input/Output Queue object
        page_queue = self._ctx.Queue()
        output_queue = self._ctx.Queue()

        # Populate the input Queue with the ids for the nodes to be read
        random.shuffle(nodes)
        for node in nodes:
            page_queue.put(node.uid)

        # Create Processes for each thread that reads the data and updates the Queue objects
        for i in range(num_threads):
            self._ctx.Process(target=target, args=(page_queue, output_queue)).start()

        # Extract the data from the output Queue object and update Page object attributes
        for i in range(len(nodes)):
            uid, attributes, out = output_queue.get()
            self.__get_node(uid).attributes.update(attributes)
            if container is not None:
                container[uid] = out

        for i in range(num_threads):
            page_queue.put(ParallelQueue.STOP)

        LOG.info('Finished %s [%s sec.]', prefix, time.time() - t)

    def _read_target(self, qin, qout):
        """Function for calling self.read with Queue objects."""
        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self.__get_node(uid)
            content = self.read(node)
            qout.put((uid, node.attributes, content))

    def _tokenize_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self.__get_node(uid)
            ast = self.tokenize(node, self._page_content[uid])
            qout.put((uid, node.attributes, ast))

    def _render_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self.__get_node(uid)
            result = self.render(node, self._page_ast[uid])
            qout.put((uid, node.attributes, result))

    def _write_target(self, qin, qout):
        """Function for calling self.write with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self.__get_node(uid)
            ast = self.write(node, self._page_result[uid])
            qout.put((uid, node.attributes, None))

    def __get_node(self, uid):
        """Helper for finding a node from the available list of page objects based on its UID."""
        for page in self._page_objects:
            if uid == page.uid:
                return page

class ParallelBarrier(Executioner):
    """
    Parallel Executioner that uses shared multiprocessing.Manager dictionaries
    for storing the Page data across processors and a parallel barrier between the
    tokenization and rendering steps to be sure that the data is completed
    for all pages prior to beginning the various steps.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._manager = self._ctx.Manager()
        self._global_attributes = self._manager.dict()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read and not self._page_content:
            self._page_content = self._manager.dict({p.uid: None for p in self._page_objects})
        if tokenize and not self._page_ast:
            self._page_ast = self._manager.dict({p.uid: None for p in self._page_objects})
        if render and not self._page_result:
            self._page_result = self._manager.dict({p.uid: None for p in self._page_objects})

        # Initialize a manager object dictionary with the current attributes of Page objects
        page_attributes = self._manager.dict({p.uid: p.attributes for p in self._page_objects})

        # Distribute nodes to threads and process the execute methods on each
        jobs = []
        random.shuffle(nodes)
        args = (self._ctx.Barrier(num_threads), page_attributes, read, tokenize, render, write)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = self._ctx.Process(target=self._target, args=(chunk, *args))
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

    def _target(self, nodes, barrier, page_attributes, read, tokenize, render, write):
        """Target function for multiprocessing.Process calls."""

        if read:
            for node in nodes:
                self._page_content[node.uid] = self.read(node)
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if tokenize:
            for node in nodes:
                node.attributes.update(page_attributes[node.uid])
                self._page_ast[node.uid] = self.tokenize(node, self._page_content[node.uid])
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if render:
            for node in nodes:
                node.attributes.update(page_attributes[node.uid])
                self._page_result[node.uid] = self.render(node, self._page_ast[node.uid])
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if write:
            for node in nodes:
                node.attributes.update(page_attributes[node.uid])
                self.write(node, self._page_result[node.uid])

    def _updateAttributes(self, page_attributes):
        """Update the local page objects with attributes gathered from the other processes"""
        for page in self._page_objects:
            mooseutils.recursive_update(page.attributes, page_attributes[page.uid])

class ParallelPipe(Executioner):
    """
    Parallel execution that performs operations and transfers data using multiprocessing.Pipe
    """
    STOP = float('inf')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._global_attributes = self._ctx.Manager().dict()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read:
            if not self._page_content:
                self._page_content = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_content, self._read_target, num_threads, "Reading")

        if tokenize:
            if not self._page_ast:
                self._page_ast = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_ast, self._tokenize_target, num_threads, "Tokenizing")

        if render:
            if not self._page_result:
                self._page_result = {p.uid: None for p in self._page_objects}
            self._run(nodes, self._page_result, self._render_target, num_threads, "Rendering")

        if write:
            self._run(nodes, None, self._write_target, num_threads, "Writing")

    def _run(self, nodes, container, target, num_threads=1, prefix='Running'):
        """Helper function for running in parallel using Pipe"""

        # Time the process
        t = time.time()
        LOG.info('%s using %s threads...', prefix, num_threads)

        # Tokenization
        jobs = []
        conn1, conn2 = self._ctx.Pipe(False)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = self._ctx.Process(target=target, args=(chunk, conn2))
            p.start()
            jobs.append(p)

        while any(job.is_alive() for job in jobs):
            if conn1.poll():
                for uid, attributes, out in conn1.recv():
                    for node in nodes:
                        if uid == node.uid:
                            node.attributes.update(attributes)
                            break
                    if container is not None:
                        container[uid] = out

        LOG.info('Finished %s [%s sec.]', prefix, time.time() - t)

    def _read_target(self, nodes, conn):
        """Function for calling self.read with Connection object"""

        data = list()
        for node in nodes:
            content = self.read(node)
            data.append((node.uid, node.attributes, content))

        with self._lock:
            conn.send(data)

    def _tokenize_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            ast = self.tokenize(node, self._page_content[node.uid])
            data.append((node.uid, node.attributes, ast))

        with self._lock:
            conn.send(data)

    def _render_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            result = self.render(node, self._page_ast[node.uid])
            data.append((node.uid, node.attributes, result))

        with self._lock:
            conn.send(data)

    def _write_target(self, nodes, conn):
        """Function for calling self.write with Connection object"""

        data = list()
        for node in nodes:
            self.write(node, self._page_result[node.uid])
            data.append((node.uid, node.attributes, None))

        with self._lock:
            conn.send(data)
