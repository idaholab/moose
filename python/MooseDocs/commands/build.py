"""Defines the MooseDocs build command."""
import os
import multiprocessing
import logging
import subprocess

import anytree
import livereload

import MooseDocs
from MooseDocs import common
from MooseDocs.tree import page

def command_line_options(subparser, parent):
    """
    Define the command line options for the build command.
    """
    parser = subparser.add_parser('build', parents=[parent],
                                  help='Convert markdown into HTML or LaTeX.')

    parser.add_argument('--config', default='config.yml',
                        help="The configuration file.")
    parser.add_argument('--destination',
                        default=os.path.join(os.getenv('HOME'), '.local', 'share', 'moose', 'site'),
                        help="Destination for writing build content.")
    parser.add_argument('--serve', action='store_true',
                        help="Create a local live server.")
    parser.add_argument('--dump', action='store_true',
                        help="Show page tree to the screen.")
    parser.add_argument('--grammer', action='store_true',
                        help='Show the lexer components in order.')
    parser.add_argument('--num-threads', '-j', type=int, default=multiprocessing.cpu_count(),
                        help="Specify the number of threads to build pages with.")
    parser.add_argument('--port', default='8000', type=str,
                        help="The local host port for live web server (default: %(default)s).")

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

        for node in anytree.PreOrderIter(self._translator.root):
            if isinstance(node, page.FileNode):
                self.watch(node.source, node.build, delay=2)

    def execute(self):
        """
        Perform complete build.
        """
        self._translator.execute(self._options.num_threads)

    def examine(self):
        """
        Investigate directories for new files and add them to the tree if found.

        TODO: Remove nodes if page is deleted.
        TODO: Handle !include (see extensions.include.py for more information).
        """
        for node in anytree.PreOrderIter(self._translator.root):

            # Only perform check on valid directories
            if not isinstance(node, page.DirectoryNode) or not os.path.exists(node.source):
                continue

            # Build map of child pages for the directory
            children = {child.name:child for child in node.children \
                        if isinstance(child, page.FileNode)}

            # Compare the list of files in the directory with those tracked by MooseDocs
            for filename in os.listdir(node.source):  #pylint: disable=no-member
                if filename.endswith(MooseDocs.FILE_EXT) and not filename.startswith('.'):
                    if filename not in children:
                        source = os.path.join(node.source, filename)
                        if filename.endswith('.md'):
                            new = page.MarkdownNode(node, source=source)
                        else:
                            new = page.FileNode(node, source=source) #pylint: disable=redefined-variable-type
                        new.base = self._options.destination
                        self.watch(new.source, new.build, delay=2) #pylint: disable=no-member
                        new.init(self._translator)
                        new.build()

        return super(MooseDocsWatcher, self).examine()

def _init_large_media():
    """Check submodule for large_media."""
    log = logging.getLogger('MooseDocs._init_large_media')
    status = common.submodule_status()
    large_media = os.path.realpath(os.path.join(MooseDocs.ROOT_DIR, 'large_media'))
    for submodule, status in status.iteritems():
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

    # Create translator
    translator = common.load_config(options.config)
    translator.init(options.destination)

    # Dump page tree
    if options.dump:
        print translator.root

    # Perform build
    translator.execute(options.num_threads)
    if options.serve:
        watcher = MooseDocsWatcher(translator, options)
        server = livereload.Server(watcher=watcher)
        server.serve(root=options.destination, port=options.port)
