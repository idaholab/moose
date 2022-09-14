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
from ..common import mixins, exceptions

LOG = logging.getLogger('MooseDocs.Executioner')

class Executioner(mixins.ConfigObject, mixins.TranslatorObject):
    """
    Base class for executing the Markdown (MooseDown) to HTML translation procedure.

    A Translator object is responsible for managing the conversion of MooseDown content to the final
    HTML result via tokenization and rendering. However, how this process executes can vary given
    the variety of strategies offered by the Python multiprocessing package, of which several have
    been deemed suitable for MooseDocs. Thus, this class was created to provide an organized and
    well-defined API for executing the translation procedure in different ways and serve as the main
    driver of MooseDocs.

    This is an internal object that should not be used directly. The Translator object should
    provide the necessary interface for all low-level reader, renderer, or extension operations. If
    access to this object is needed for any purpose except to reference shared memory containers or
    processor locks, which should be avoided, then the Translator is not designed correctly.

    ***TRANSLATION PROCESSING***
    Significant effort has been made to correctly handle the data involved with translation using
    the multiprocessing package. Throughout the development of MooseDocs, the algorithm used has
    changed in an attempt to optimize performance. There has not been one method that has been ideal
    for all cases, so rather than be restricted to a single algorithm the Executioner system was
    created to allow for the algorithm to be changed as well as new ones to be developed.

    The exact translation process varies between Executioner objects and further depends on the
    inputs they are constructed with, but all proceed through the following basic steps:

        1. Execute preExecute methods
        2. Read raw MooseDown content from pages.Source objects
           2.1. Execute preRead methods
           2.2. Perform read using Reader.read method
           2.3. Execute postRead methods
        3. Tokenize pages.Source objects
           3.1. Execute preTokenize methods
           3.2. Perform tokenization using Reader.tokenize method
           3.3. Execute postTokenize methods
        4. Render pages.Source objects
           4.1. Execute preRender methods
           4.2. Perform rendering using Renderer.render method
           4.3. Execute postRender methods
        5. Write pages.Source objects
           5.1. Execute preWrite methods
           5.2. Perform write, using Renderer.write method
           5.3. Execute postWrite methods
        6. Finalize remaining content, i.e., copy pages.File objects to Translator.destination
        7. Execute postExecute methods

    Many MooseDocs objects like readers, renderers, and extensions define pre/post methods like
    postTokenize() or preRender(). The Translator object is responsible for invoking all matching
    occurrences of these methods upon request by this class. The read(), tokenize(), render(), and
    write() methods belong only to either the Reader or Renderer object, and tokenize() and render()
    perform the especially important task of invoking (via a trace of stack entries) the key
    functions of the Component objects defined in extension modules, such as createToken() or
    createHTML(). All of these distinct methods provide a lot of flexibility about where, when, and
    how content is received and translated.

    The various methods of this class and all those of its subclasses are designed to be invoked any
    number of times while the MooseDocs application is running and on any subset of Page objects.
    The main purpose of this design is to support translation of existing pages edited, or new pages
    added, during a live reload (see the MooseDocsWatcher class in 'commands/build.py'). This design
    also makes it possible to call the methods to perform specific steps of the translation process,
    e.g., to perform only the read and tokenize steps. The rule, though, is that successive
    invocations must follow the basic procedure: read -> tokenize -> render -> write. For example,
    if only the read step has been performed, the Executioner must be called to tokenize before it
    can be called to render. The progress checks that enforce this rule are reset during the write
    step, and the entire procedure must be repeated if called upon thereafter.

    ***PERFORMANCE REVIEW, SEPTEMBER 2021***
    The table below presents the results of a procedure to benchmark each of the Executioner classes
    provided by this module. The methodology consisted of executing the MooseDocs application
    multiple times per class: A dry run intended to initialize the Python JIT compilers and prepare
    buffer/cache memory for the write step, followed by two consecutive control runs for which the
    peak amount of RAM consumed by, and duration of, translation steps were recorded. The results
    reported are the mean averages of the two control runs. For parallel executioners, the procedure
    was conducted with both 4 and 8 CPUs. Prior to each test case, the system was rebooted to help
    control for potential interference by accumulated memory leaks or background processes.

    The specimen involved was the primary configuration of the MOOSE website, which includes the
    content for all physics modules but not any of the slideshows (e.g., the workshop). For example,
    a single run of the "ParallelBarrier [x8]" test case was executed using the following Bash
    commands from the root directory of the MOOSE repository:

        ```
        cd modules/doc
        ./moosedocs.py build -j8 --config config.yml --executioner MooseDocs.base.ParallelBarrier
        ```

    The apparatus was a Dell XPS 8940 with Ubuntu 20.04.3 LTS (64-bit) as the OS. The relevant
    system specifications are as follows:
        - Processor: Intel Core i7-11700K (8-Core, 16 CPUs, 16M Cache, 3.6-5.0GHz)
        - Memory: 32GB (16Gx2), DDR4, 2933MHz
        - Disk: 1TB M.2 PCIe NVME SSD

    It is important to note that the 'build' command disabled the SQA, Civet, and GitUtils
    extensions in all runs in accordance with its default behavior. Also note that, with the
    exception of the Serial class, the executioners randomize the order of the Page objects in the
    input list of those to be translated. However, this task was not performed in any of the test
    cases (by temporarily removing the corresponding lines of code) for the sake of fairness.

                           +---------------------+---------------------+----------+
                           |    READ/TOKENIZE    |    RENDER/WRITE     |  TOTAL*  |
    +----------------------+----------+----------+----------+----------+----------+
    |EXECUTIONER [CPUs]    |  RAM (GB)|  TIME (s)|  RAM (GB)|  TIME (s)|  TIME (s)|
    +----------------------+----------+----------+----------+----------+----------+
    |Serial                |       1.2|      75.2|       3.7|     161.2|     243.4|
    +----------------------+----------+----------+----------+----------+----------+
    |ParallelQueue [x4]    |       2.3|      35.4|       9.8|      76.7|     119.0|
    |ParallelQueue [x8]    |       2.7|      33.4|      11.7|      63.4|     103.9|
    +----------------------+----------+----------+----------+----------+----------+
    |ParallelBarrier [x4]  |       1.9|      53.8|       7.4|     121.0|     182.0|
    |ParallelBarrier [x8]  |       2.8|      47.7|       8.9|     113.1|     168.0|
    +----------------------+----------+----------+----------+----------+----------+
    |ParallelPipe [x4]     |       1.4|      48.5|       6.9|      99.8|     155.2|
    |ParallelPipe [x8]     |       1.5|      41.2|       9.5|      88.0|     136.2|
    +----------------------+----------+----------+----------+----------+----------+
        *Includes the durations of the preExecute, finalize, and postExecute steps.

    The data given above benchmarks the complete content of the main MOOSE website consisting of
    2,780 pages.Source objects as of September 8, 2021. The mean execution times will vary and most
    likely increase indefinitely over time as more content is added. However, it seems reasonable to
    assume that the difference in execution times between any two executioners would scale
    proportionally to those indicated here. The fastest executioner for any given configuration
    will depend on the amount and content being translated, as well as perhaps the computer running
    the application. But if the main MOOSE website content is assumed to represent the typical build
    job, then the data suggests that the ParallelQueue executioner is the highest performing one
    and, thus, should be the default.

    Each Executioner class has its advantages. For example, although ParallelPipe is not as fast as
    ParallelQueue in this study, it consumes less RAM, which becomes a precious resource as the
    amount required for the render and write steps approaches the usual amount available on economic
    workstations of 16 GB. ParallelBarrier seems to be the slowest parallel type, but it performs
    garbage collection of shared memory containers during runtime - a task that other executioners
    spend an amount of time doing not shown here while MooseDocs shuts down. As yet another example,
    the Reveal type renderer currently combines all slides into a single HTML page, and so it makes
    sense to use the Serial executioner for these configurations. Thus, the default executioner
    should be determined on a case-by-case basis, and benchmarking for all should be done following
    periods of significant content additions and/or updates in MooseDocs.

    TODO: As mentioned, execution times will most likely increase in the future. The build job for
    the main MOOSE website will only become more and more expensive to the point where the current
    infrastructure is no longer practical. For this imminent scenario, two solutions are proposed:
        1) Develop an executioner that makes use of the concurrent.futures Python module (see
           https://docs.python.org/3/library/concurrent.futures.html), which may provide
           capabilities and performance levels that are superior to the multiprocessing package.
        2) Cache the resulting site folder, either on a repository or from the initial build job a
           user runs (as it already does this but for EACH build job), and develop a change tracking
           system based on diffs or timestamps of the content files so it is known which need to be
           re-translated. Then, build only those Page objects as well as any of their dependencies.
    """

    @staticmethod
    def defaultConfig():
        config = mixins.ConfigObject.defaultConfig()
        config['profile'] = (False, "Run the executioner with profiling.")
        return config

    def __init__(self, **kwargs):
        mixins.ConfigObject.__init__(self, **kwargs)
        mixins.TranslatorObject.__init__(self)
        self._page_objects = dict()
        self._total_time = 0
        self._clear_progress()

        # The method for spawning processes changed to "spawn" for macOS in python 3.9. Currently,
        # MooseDocs does not work with this combination.
        # Ints are used for the second comparison because simple string comparison is inadequate against 3.9 for >= 3.10
        if (platform.python_version_tuple()[0] >= '3') and (int(platform.python_version_tuple()[1]) >= 9) and (platform.system() == 'Darwin'):
            self._ctx = multiprocessing.get_context('fork')
        else:
            self._ctx = multiprocessing.get_context()

        # A lock used prior to caching items or during directory creation to prevent race conditions
        self._lock = self._ctx.Lock()

        # Containers for storing the ouputs from the read, tokenize, and render methods, as well as
        # another for storing miscellaneous data, respectively. For parallel executioners, these may
        # need to be reset as shared-memory multiprocessing ('self._ctx') Manager().dict() objects
        # so that modifications are properly handled and communicated across processors.
        self._page_content = dict()
        self._page_ast = dict()
        self._page_result = dict()
        self._global_attributes = dict()

    def initPages(self, nodes):
        """Initialize the Page objects."""

        # Initialize Page objects
        for node in nodes:
            # Assign translator instance, destination root, and output extension
            node.translator = self.translator
            node.base = self.translator.destination
            if isinstance(node, pages.Source):
                node.output_extension = self.translator.renderer.EXTENSION

            # Setup page attributes
            for ext in self.translator.extensions:
                node.attributes['__{}__'.format(ext.name)] = dict()
            self.translator.executePageMethod('initPage', node)

    def addPage(self, page):
        """Add a Page object to be Translated."""
        self._page_objects[page.uid] = page

    def getPages(self):
        """Return a list of Page objects."""
        return self._page_objects.values()

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
            (read, tokenize, render, write)[bool]: The tasks to perform (see the class description).
        """
        raise NotImplementedError("The execute method must be defined.")

    def __call__(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        """
        Called by Translator object - this executes the steps listed in the class description.
        """
        initial_time = time.time()

        if read:
            self.translator.executeMethod('preExecute', log=True)

        source_nodes = [n for n in nodes if isinstance(n, pages.Source)]
        if source_nodes:
            translate_steps = self._setupCall(source_nodes, read, tokenize, render, write)
            if translate_steps:
                t = time.time()
                n = len(source_nodes) if num_threads > len(source_nodes) else num_threads
                if isinstance(self, MooseDocs.base.Serial):
                    LOG.info('Translating (%s) in serial...', translate_steps)
                else:
                    LOG.info('Translating (%s) using %s threads...', translate_steps, n)
                self.execute(source_nodes, n, read, tokenize, render, write)
                LOG.info('Translating complete [%s sec.]', time.time() - t)

        # Indexing/copying
        if write:
            other_nodes = [n for n in nodes if not isinstance(n, pages.Source)]
            if other_nodes:
                LOG.info('Copying content...')
                t = self.finalize(other_nodes)
                LOG.info('Copying complete [%s sec.]', t)

            self.translator.executeMethod('postExecute', log=True)

            LOG.info('Total Time [%s sec.]', self._total_time + time.time() - initial_time)
            self._total_time = 0 # reset the execution timer
        else:
            self._total_time += time.time() - initial_time

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
            self.translator.executePageMethod('preTokenize', node, args=(content, ast))
            if node.attributes.get('tokenize', True):
                self.translator.reader.tokenize(ast, content, node)
            self.translator.executePageMethod('postTokenize', node, args=(ast,))

        return ast

    def render(self, node, ast):
        """Perform rendering and call all associated callbacks for the supplied page."""

        result = self.translator.renderer.getRoot()
        if node.attributes.get('active', True):
            self.translator.executePageMethod('preRender', node, args=(ast, result))
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

    def finalize(self, other_nodes):
        """Complete copying of non-source (e.g., images) files."""
        start = time.time()
        for node in other_nodes:
            self.translator.renderer.write(node)
        return time.time() - start

    def _clear_progress(self):
        """Set the translation progress trackers to False."""
        self._has_read = False
        self._has_tokenized = False
        self._has_rendered = False

    def _setupCall(self, nodes, read, tokenize, render, write):
        """
        Helper that initializes the page output containers and asserts that the translation steps
        are executed in the correct order, i.e., Read -> Tokenize -> Render -> Write.

        This method returns a string expressing the steps being performed if any, e.g., 'reading' or
        'reading, tokenizing, and rendering', to be included in LOG.info() reports.
        """
        def calloc(nodes, container):
            uids = container.keys() # need only grab current uid keys, leave this out of node loop!!
            for node in nodes:
                if node.translator is self.translator:
                    if node.uid not in uids:
                        container[node.uid] = None
                else:
                    msg = "Bad translator reference for {} object with local name '{}'. Please " \
                          "call the execute() method of the assosciated Translator instance."
                    raise exceptions.MooseDocsException(msg, str(node).split(':')[0], node.local)

        msg = 'The "{}" translation step must be executed before the "{}" step.'
        steps = list()
        if read:
            calloc(nodes, self._page_content)
            steps.append('reading')
            self._has_read = True

        if tokenize:
            if not self._has_read:
                raise exceptions.MooseDocsException(msg, "read", "tokenize")
            calloc(nodes, self._page_ast)
            steps.append('tokenizing')
            self._has_tokenized = True

        if render:
            if not self._has_tokenized:
                raise exceptions.MooseDocsException(msg, "tokenize", "render")
            calloc(nodes, self._page_result)
            steps.append('rendering')
            self._has_rendered = True

        if write:
            if not self._has_rendered:
                raise exceptions.MooseDocsException(msg, "render", "write")
            steps.append('writing')
            self._clear_progress()

        # If translation steps are being performed, return a string expressing those which are
        if len(steps) > 2:
            steps[-1] = 'and ' + steps[-1]
            return ', '.join(steps)
        elif steps:
            return ' and '.join(steps)

    def _getPage(self, uid):
        """Retrieve a Page object from the global list by its UID."""
        return self._page_objects[uid]

class Serial(Executioner):
    """Simple serial Executioner, this is useful for debugging or for very small builds."""

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read:
            self._run(nodes, self._readHelper)
        if tokenize:
            self._run(nodes, self._tokenizeHelper)
        if render:
            self._run(nodes, self._renderHelper)
        if write:
            self._run(nodes, self._writeHelper)

    def _run(self, nodes, target):
        """Run and optionally profile a function"""

        if self['profile']:
            mooseutils.run_profile(target, nodes)
        else:
            target(nodes)

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
            self._run(nodes, self._page_content, self._read_target, num_threads)
        if tokenize:
            self._run(nodes, self._page_ast, self._tokenize_target, num_threads)
        if render:
            self._run(nodes, self._page_result, self._render_target, num_threads)
        if write:
            self._run(nodes, None, self._write_target, num_threads)

    def _run(self, nodes, container, target, num_threads=1):
        """Helper function for running in parallel using Queues"""

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
            self._getPage(uid).attributes.update(attributes)
            if container is not None:
                container[uid] = out

        for i in range(num_threads):
            page_queue.put(ParallelQueue.STOP)

    def _read_target(self, qin, qout):
        """Function for calling self.read with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._getPage(uid)
            content = self.read(node)
            qout.put((uid, node.attributes, content))

    def _tokenize_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._getPage(uid)
            ast = self.tokenize(node, self._page_content[uid])
            qout.put((uid, node.attributes, ast))

    def _render_target(self, qin, qout):
        """Function for calling self.tokenize with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._getPage(uid)
            result = self.render(node, self._page_ast[uid])
            qout.put((uid, node.attributes, result))

    def _write_target(self, qin, qout):
        """Function for calling self.write with Queue objects."""

        for uid in iter(qin.get, ParallelQueue.STOP):
            node = self._getPage(uid)
            ast = self.write(node, self._page_result[uid])
            qout.put((uid, node.attributes, None))

class ParallelBarrier(Executioner):
    """
    Parallel Executioner that uses a shared multiprocessing.Manager().dict() container to store page
    attributes gathered across processors and a multiprocessing.Barrier object to synchronize the
    translation procedure. Once all processors complete a translation step, they're simultaneously
    released and immediately update their local copies of the 'self._page_objects' container before
    proceeding to the following step.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._manager = self._ctx.Manager()
        self._page_content = self._manager.dict()
        self._page_ast = self._manager.dict()
        self._page_result = self._manager.dict()
        self._global_attributes = self._manager.dict()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        page_attributes = self._manager.dict({p.uid: p.attributes for p in nodes})

        # Distribute nodes to Barrier objects and run the _target() method on each.
        jobs = []
        random.shuffle(nodes)
        args = (self._ctx.Barrier(num_threads), page_attributes, read, tokenize, render, write)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            p = self._ctx.Process(target=self._target, args=(chunk, *args))
            jobs.append(p)
            p.start()

        for job in jobs:
            job.join()

        # The original copy of the 'self.page_objects' container needs to be updated to ensure that
        # the class instance retains this information over succesive invocations of this method.
        self._updateAttributes(page_attributes)

    def _target(self, nodes, barrier, page_attributes, read, tokenize, render, write):
        """Target function for multiprocessing.Process() calls."""

        if read:
            for node in nodes:
                self._page_content[node.uid] = self.read(node)
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if tokenize:
            for node in nodes:
                mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
                self._page_ast[node.uid] = self.tokenize(node, self._page_content[node.uid])
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if render:
            for node in nodes:
                mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
                self._page_result[node.uid] = self.render(node, self._page_ast[node.uid])
                page_attributes[node.uid] = node.attributes

            barrier.wait()
            self._updateAttributes(page_attributes)

        if write:
            for node in nodes:
                mooseutils.recursive_update(node.attributes, page_attributes[node.uid])
                self.write(node, self._page_result[node.uid])

    def _updateAttributes(self, page_attributes):
        """Update the Page object attributes with those gathered from all processes."""
        for uid, attributes in page_attributes.items():
            self._getPage(uid).attributes.update(attributes)

class ParallelPipe(Executioner):
    """
    Parallel execution that performs operations and transfers data using multiprocessing.Pipe

    Follows Pipe example with wait():
    https://docs.python.org/3/library/multiprocessing.html#multiprocessing.connection.wait

    Note: 'ForkContext' objects do not have the attribute 'connection'. However,
    `multiprocessing.connection.wait(receivers) == [r for r in receivers if r.poll() or r.closed]`
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._global_attributes = self._ctx.Manager().dict()

    def execute(self, nodes, num_threads=1, read=True, tokenize=True, render=True, write=True):
        if read:
            self._run(nodes, self._page_content, self._read_target, num_threads)
        if tokenize:
            self._run(nodes, self._page_ast, self._tokenize_target, num_threads)
        if render:
            self._run(nodes, self._page_result, self._render_target, num_threads)
        if write:
            self._run(nodes, None, self._write_target, num_threads)

    def _run(self, nodes, container, target, num_threads=1):
        """Helper function for running in parallel using Pipe"""

        # Create connection objects representing the receiver and sender ends of the pipe.
        receivers = []
        random.shuffle(nodes)
        for chunk in mooseutils.make_chunks(nodes, num_threads):
            r, s = self._ctx.Pipe(False)
            receivers.append(r)
            self._ctx.Process(target=target, args=(chunk, s)).start()
            s.close() # need to close this instance because a copy was sent to the Process() object

        # Iterate through the list of ready connection objects, i.e., those that either have data to
        # receive or their corresponding sender connection has been closed, until all are removed
        # from the list of pending connections. If there is no data to receive and the sender has
        # been closed, then an EOFError is raised indicating that the receiver can be removed.
        while receivers:
            for r in [r for r in receivers if r.poll() or r.closed]:
                try:
                    data = r.recv()
                except EOFError:
                    receivers.remove(r)
                else:
                    for uid, attributes, out in data:
                        self._getPage(uid).attributes.update(attributes)
                        if container is not None:
                            container[uid] = out

    def _read_target(self, nodes, conn):
        """Function for calling self.read with Connection object"""

        data = list()
        for node in nodes:
            content = self.read(node)
            data.append((node.uid, node.attributes, content))

        conn.send(data)

    def _tokenize_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            ast = self.tokenize(node, self._page_content[node.uid])
            data.append((node.uid, node.attributes, ast))

        conn.send(data)

    def _render_target(self, nodes, conn):
        """Function for calling self.tokenize with Connection object"""

        data = list()
        for node in nodes:
            result = self.render(node, self._page_ast[node.uid])
            data.append((node.uid, node.attributes, result))

        conn.send(data)

    def _write_target(self, nodes, conn):
        """Function for calling self.write with Connection object"""

        data = list()
        for node in nodes:
            self.write(node, self._page_result[node.uid])
            data.append((node.uid, node.attributes, None))

        conn.send(data)
