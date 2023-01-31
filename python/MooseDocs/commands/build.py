#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the MooseDocs build command."""
import os
import sys
import time
import collections
import multiprocessing
import logging
import subprocess
import shutil
import yaml
import livereload
import mooseutils
from mooseutils.yaml_load import yaml_load

import MooseDocs
from .. import common
from ..tree import pages

LOG = logging.getLogger('MooseDocs.build')

def command_line_options(subparser, parent):
    """
    Define the command line options for the build command.
    """
    parser = subparser.add_parser('build', parents=[parent],
                                  help='Convert markdown into HTML or LaTeX.')

    parser.add_argument('--config', nargs='+',
                        default=[file for file in os.listdir() if file.endswith('config.yml')],
                        help="The list of configuration files " \
                             "(default: all filenames that end with 'config.yml').")
    parser.add_argument('--args', default=None, type=lambda a: yaml.load(a, yaml.Loader),
                        help="YAML content to override configuration items supplied in file.")
    parser.add_argument('--disable', nargs='*', default=[],
                        help="A list of extensions to disable.")
    parser.add_argument('--fast', action='store_true',
                        help="Build the pages with the slowest extension (appsyntax) disabled.")
    parser.add_argument('--executioner',
                        help="Select the mode of execution " \
                             "(default: MooseDocs.base.ParallelQueue).")
    parser.add_argument('--profile', action='store_true',
                        help="Build the pages with python profiling.")
    parser.add_argument('--destination', default=None,
                        help="Destination for writing build content.")
    parser.add_argument('--serve', action='store_true',
                        help="Create a local live server.")
    parser.add_argument('--dump', action='store_true',
                        help="Show page tree to the screen.")
    parser.add_argument('--num-threads', '-j', type=int, choices=range(1,13),
                        default=int(min(multiprocessing.cpu_count() / 2, 4)),
                        help="The number of parallel threads to execute with. The max. is twelve.")
    parser.add_argument('--port', default='8000', type=str,
                        help="The host port for live web server (default: %(default)s).")
    parser.add_argument('--host', default='127.0.0.1', type=str,
                        help="The local host for live web server (default: %(default)s).")
    parser.add_argument('--clean', type=str, choices=['0', 'false', 'no', '1', 'true', 'yes'],
                        help="Clean the destination directory, by default this is False  when " \
                             "the '--files' option is used, otherwise the default is True.")
    parser.add_argument('-f', '--files', default=[], nargs='*',
                        help="A list of files and/or directories to build - this is useful for " \
                             "testing. All pages whose local names begin with one of the given " \
                             "strings will be built.")
    parser.add_argument('--stable', '--with-sqa', action='store_true',
                        help="By default the CIVET and SQA related extensions are disabled " \
                        "because they are slow and require the use of dependent Git submodules. " \
                        "These extensions are intended for use on the stable website only. " \
                        "This flag will enable those extensions and override use of --fast.")
    parser.add_argument('--home', default=None, help="The 'home' URL for the hosted website. " \
                                                     "This is mainly used by CIVET to allow " \
                                                     "temporary sites to be functional.")
    parser.add_argument('--hide-source', action='store_true',
                        help="Shortcut for setting the 'hide_source' option in the modal extension.")

class MooseDocsWatcher(livereload.watcher.Watcher):
    """
    A livereload watcher for MooseDocs that rebuilds or adds pages as needed during a live serve.
    """

    def __init__(self, translators, contents, configurations, num_threads):
        super(MooseDocsWatcher, self).__init__()

        self._items = list()
        self._contents = contents
        self._num_threads = num_threads
        self._primary = translators[0] # only used when not building multiple configs

        roots = set()
        for translator, config in zip(translators, configurations):
            items = common.get_items(config['Content'])
            self._items.append(items)

            # Compile the set of directories to watch
            for root, _, _ in common.get_files(items, translator.reader.EXTENSIONS):
                roots.add(root)

        for root in roots:
            self.watch(root, self.build, delay=1)

    def build(self):
        """Build the necessary pages based on the current filepath."""

        # Locate page object to be translated, return if nothing is found
        page = self._getPage(self.filepath)
        if page is None:
            return
        MooseDocs.PROJECT_FILES.add(self.filepath)

        # Build lists of pages the current one depends on and map them to their translators
        nodes = {page.translator: [page]}
        for node in self._contents:
            if page.uid in node.get('dependencies', set()):
                nodes[node.translator] = nodes.get(node.translator, list()) + [node]

        # Execute the read and tokenize methods on all translators
        for translator, content in nodes.items():
            translator.execute(content, self._num_threads, render=False, write=False)

        # Finally, execute the render and write methods
        for translator, content in nodes.items():
            translator.execute(content, self._num_threads, read=False, tokenize=False)

    def _getPage(self, source):
        """Search the existing content for pages, if it doesn't exist create it."""
        exists = os.path.isfile(source) # there are different ways to handle case of deleted files
        for page in self._contents:
            if source == page.source:
                if exists:
                    return page
                else: # remove from content pool so we don't attempt to translate a nonexistent file
                    self._contents.remove(page)
                    return

        # Create a new page object if dealing with only one translator here
        if len(self._items) == 1 and exists:
            for root, fname, _ in common.get_files(self._items[0], self._primary.reader.EXTENSIONS):
                if source == fname:
                    key = fname.replace(root, '').strip('/')
                    page = common.create_file_page(key, fname, self._primary.reader.EXTENSIONS)
                    self._primary.addPage(page)
                    self._primary.initPage(page)
                    self._contents.append(page)
                    return page
        elif exists: # there is no way to know which translator should build the new content
            msg = "The new file '%s' cannot be translated since there are multiple " \
                  "configurations. Use '--help' for more information."
            LOG.warning(msg, os.path.relpath(source, MooseDocs.ROOT_DIR))

def main(options):
    """
    Main function for the build command.

    Inputs:
        options[argparse options]: Complete options from argparse, see MooseDocs/main.py
    """
    t = time.time()

    # Infinite nested dict
    tree = lambda: collections.defaultdict(tree)
    kwargs = tree()

    # Setup executioner
    if options.executioner:
        kwargs['Executioner']['type'] = options.executioner

    # Disable extensions
    if options.stable:
        pass
    elif options.fast:
        options.disable += ['MooseDocs.extensions.appsyntax', 'MooseDocs.extensions.navigation',
                            'MooseDocs.extensions.sqa', 'MooseDocs.extensions.civet',
                            'MooseDocs.extensions.gitutils']
    else:
        options.disable += ['MooseDocs.extensions.sqa', 'MooseDocs.extensions.civet',
                            'MooseDocs.extensions.gitutils']

    for name in options.disable:
        kwargs['Extensions'][name] = dict(active=False)

    if options.hide_source:
        kwargs['Extensions']['MooseDocs.extensions.modal']['hide_source'] = True

    # Apply Translator settings
    if options.destination:
        kwargs['Translator']['destination'] = mooseutils.eval_path(options.destination)
    if options.profile:
        kwargs['Translator']['profile'] = True

    # Apply '--args' and override anything already set
    if options.args is not None:
        mooseutils.recursive_update(kwargs, options.args)

    # Create translators for the specified configuration files, provide kwargs to override them
    config_files = options.config if isinstance(options.config, list) else [options.config]
    subconfigs = len(config_files) > 1
    LOG.info("Loading configuration file{}".format('s' if subconfigs else ''))
    translators, contents, configurations = common.load_configs(config_files, **kwargs)

    # Initialize the translator objects
    for index, translator in enumerate(translators):
        if subconfigs:
            LOG.info('Initializing translator object loaded from %s', config_files[index])
        translator.init(contents[index])

        # init methods can add pages (e.g., civet.py), but don't add pages from another translator
        contents[index] = [page for page in translator.getPages() if page.translator is translator]

    # Identify the first translator in the list as the "primary" one for convenience
    primary = translators[0]
    pooled = sorted(primary.getPages(), key=(lambda p: p.local))

    # Dump page tree from content pool and syntax list from all translators with AppSyntax
    if options.dump:
        for page in pooled:
            print('{}: {}'.format(page.local, page.source))
        for index, translator in enumerate(translators):
            for extension in translator.extensions:
                if isinstance(extension, MooseDocs.extensions.appsyntax.AppSyntaxExtension):
                    if subconfigs:
                        LOG.info('Building syntax list specified by %s', config_files[index])
                    extension.preExecute()
                    print(extension.syntax)
                    break
        return 0

    # TODO: See `navigation.postExecute`
    #       The navigation "home" should be a markdown file, when all the apps update to this we
    #       can remove this as well as the use of it by CIVET
    home = options.home
    if options.serve and (home is not None) and (not home.endswith('.md')):
        home = 'http://127.0.0.1:{}'.format(options.port)

    if home is not None:
        for ext in primary.extensions:
            if 'home' in ext:
                ext.update(home=home)

    # Set default for --clean: clean when --files is NOT used.
    if options.clean is None:
        options.clean = options.files == []
    else:
        options.clean = options.clean.lower() in ['true', 'yes', '1']

    if options.clean and os.path.exists(primary.destination):
        LOG.info("Cleaning destination %s", primary.destination)
        shutil.rmtree(primary.destination)

    # Update contents lists if only building certain files
    if options.files:
        for index, translator in enumerate(translators):
            func = lambda p: (p in contents[index]
                              and any([p.local.startswith(f) for f in options.files]))
            contents[index] = translator.findPages(func)

    # Execute the read and tokenize methods on all translators
    for index, translator in enumerate(translators):
        if subconfigs:
            LOG.info('Reading content specified by %s', config_files[index])
        translator.execute(contents[index], num_threads=options.num_threads, render=False, write=False)

    # Finally, execute the render and write methods
    for index, translator in enumerate(translators):
        if subconfigs:
            LOG.info('Writing content specified by %s', config_files[index])
        translator.execute(contents[index], num_threads=options.num_threads, read=False, tokenize=False)

    LOG.info('Total Time [%s sec.]', time.time() - t)

    # Run live server and watch for content changes
    if options.serve:
        watcher = MooseDocsWatcher(translators, pooled, configurations, options.num_threads)
        server = livereload.Server(watcher=watcher)
        server.serve(root=primary.destination, host=options.host, port=options.port)

    return 0
