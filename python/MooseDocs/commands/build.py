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
import multiprocessing
import logging
import subprocess
import shutil
import livereload
import mooseutils
from mooseutils.yaml_load import yaml_load
import MooseDocs
from MooseDocs import common
from MooseDocs.tree import pages
from .check import check

def command_line_options(subparser, parent):
    """
    Define the command line options for the build command.
    """
    parser = subparser.add_parser('build', parents=[parent],
                                  help='Convert markdown into HTML or LaTeX.')

    parser.add_argument('--config', default='config.yml',
                        help="The configuration file.")
    parser.add_argument('--disable', nargs='*', default=[],
                        help="A list of extensions to disable.")
    parser.add_argument('--fast', action='store_true',
                        help="Build the pages with the slowest extension (appsyntax) disabled.")
    parser.add_argument('--executioner',
                        help="Select the mode of execution " \
                             "(default: MooseDocs.base.ParallelBarrier).")
    parser.add_argument('--profile', action='store_true',
                        help="Build the pages with python profiling.")
    parser.add_argument('--destination',
                        default=None,
                        help="Destination for writing build content.")
    parser.add_argument('--serve', action='store_true',
                        help="Create a local live server.")
    parser.add_argument('--dump', action='store_true',
                        help="Show page tree to the screen.")
    parser.add_argument('--grammar', action='store_true',
                        help='Show the lexer components in order.')
    parser.add_argument('--num-threads', '-j', type=int, default=multiprocessing.cpu_count(),
                        help="Specify the number of threads to build pages with.")
    parser.add_argument('--port', default='8000', type=str,
                        help="The host port for live web server (default: %(default)s).")
    parser.add_argument('--host', default='127.0.0.1', type=str,
                        help="The local host for live web server (default: %(default)s).")
    parser.add_argument('--clean', type=str, choices=['0', 'false', 'no', '1', 'true', 'yes'],
                        help="Clean the destination directory, by default this is False  when " \
                             "the '--files' option is used, otherwise the default is True.")
    parser.add_argument('-f', '--files', default=[], nargs='*',
                        help="A list of file to build, this is useful for testing. The paths " \
                             "should be as complete as necessary to make the name unique, just " \
                             "as done within the markdown itself.")
    parser.add_argument('--home', default=None, help="The 'home' URL for the hosted website. " \
                                                     "This is mainly used by CIVET to allow " \
                                                     "temporary sites to be functional.")

    parser.add_argument('--check', action='store_true',
                        help="Run the default check command prior to build, the main purpose " \
                             "of this command is to allow the make targets to avoid creating " \
                             "the syntax multiple times.")

class MooseDocsWatcher(livereload.watcher.Watcher):
    """
    A livereload watcher for MooseDocs that adds nodes to the directory tree when pages are added.

    Inputs:
        translator[Translator]: Instance of the translator object for converting files.
        options[argparse]: Complete argparse options as passed into the main function.
    """

    def __init__(self, translator, options, *args, **kwargs):

        super(MooseDocsWatcher, self).__init__(*args, **kwargs)
        self._options = options
        self._translator = translator

        self._config = yaml_load(options.config, root=MooseDocs.ROOT_DIR)

        # Determine the directories to watch
        roots = set()
        self._items = common.get_items(self._config.get('Content'))
        for root, _ in common.get_files(self._items, self._translator.reader.EXTENSIONS):
            roots.add(root)

        for root in roots:
            self.watch(root, self.build, delay=1)

    def build(self):
        """Build the necessary pages based on the current filepath."""

        # Locate the page to be translated
        page = self._getPage(self.filepath)
        if page is None:
            return
        MooseDocs.PROJECT_FILES.add(self.filepath)

        # Build a list of pages to be translated including the dependencies
        nodes = [page]
        for node in self._translator.content:
            uids = self._translator.getMetaData(node, 'dependencies') or []
            if page.uid in uids:
                nodes.append(node)

        self._translator.execute(self._options.num_threads, nodes)

    def _getPage(self, source):
        """Search the existing content for pages, if it doesn't exist create it."""

        # Search for the page based on the source name, if it is found return the page
        for page in self._translator.content:
            if source == page.source:
                return page

        # Build a list of all filenames
        filenames = common.get_files(self._items, self._translator.reader.EXTENSIONS, False)

        # Build a page object if the filename shows up in the list of available files
        for root, filename in filenames:
            if filename == source:
                key = filename.replace(root, '').strip('/')
                page = common.create_file_page(key, filename, self._translator.reader.EXTENSIONS)
                page.base = self._translator.get('destination')
                if isinstance(page, pages.Source):
                    page.output_extension = self._translator.renderer.EXTENSION
                self._translator.addContent(page)
                return page

def _init_large_media():
    """Check submodule for large_media."""
    log = logging.getLogger('MooseDocs._init_large_media')
    status = common.submodule_status()
    large_media = os.path.realpath(os.path.join(MooseDocs.MOOSE_DIR, 'large_media'))
    for submodule, status in status.items():
        if ((os.path.realpath(os.path.join(MooseDocs.MOOSE_DIR, submodule)) == large_media)
                and (status == '-')):
            log.info("Initializing the 'large_media' submodule for storing images above 1MB.")
            subprocess.call(['git', 'submodule', 'update', '--init', 'large_media'],
                            cwd=MooseDocs.MOOSE_DIR)

def main(options):
    """
    Main function for the build command.

    Inputs:
        options[argparse options]: Complete options from argparse, see MooseDocs/main.py
    """

    # Make sure "large_media" exists in MOOSE
    _init_large_media()

    # Setup executioner
    kwargs = dict()
    if options.executioner:
        kwargs['Executioner'] = {'type':options.executioner}

    # Create translator
    translator, _ = common.load_config(options.config, **kwargs)
    if options.destination:
        translator.update(destination=mooseutils.eval_path(options.destination))
    if options.profile:
        translator.executioner.update(profile=True)
    translator.init()

    # Disable slow extensions for --fast
    if options.fast:
        options.disable.append('appsyntax')
        options.disable.append('navigation')
        options.disable.append('sqa')

    # Disable extensions based on command line arguments
    if options.disable:
        for ext in translator.extensions:
            if ext.name in options.disable: #pylint: disable=protected-access
                ext.setActive(False)

    # Replace "home" with local server
    home = options.home
    if options.serve:
        home = 'http://127.0.0.1:{}'.format(options.port)
    if home is not None:
        for ext in translator.extensions:
            if 'home' in ext:
                ext.update(home=home, set_initial=True)

    # Dump page tree
    if options.dump:
        for page in translator.content:
            print('{}: {}'.format(page.local, page.source))
        sys.exit()

    # Set default for --clean: clean when --files is NOT used.
    if options.clean is None:
        options.clean = options.files == []
    else:
        options.clean = options.clean.lower() in ['true', 'yes', '1']

    if options.clean and os.path.exists(translator['destination']):
        log = logging.getLogger('MooseDocs.build')
        log.info("Cleaning destination %s", translator['destination'])
        shutil.rmtree(translator['destination'])

    # Perform check
    if options.check:
        check(translator)

    # Perform build
    if options.files:
        nodes = []
        for filename in options.files:
            nodes += translator.findPages(filename)
        translator.execute(options.num_threads, nodes)
    else:
        translator.execute(options.num_threads)

    if options.serve:
        watcher = MooseDocsWatcher(translator, options)
        server = livereload.Server(watcher=watcher)
        server.serve(root=translator['destination'], host=options.host, port=options.port)

    return 0
