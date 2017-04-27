import os
import sys
import glob
import re
import livereload
import multiprocessing
import logging
import shutil
import MooseDocs
import build
log = logging.getLogger(__name__)

def serve_options(parser):
    """
    Command-line options for serve command.
    """
    build.build_options(parser, site_dir=False)
    parser.add_argument('--host', default='127.0.0.1', type=str, help="The local host location for live web server (default: %(default)s).")
    parser.add_argument('--port', default='8000', type=str, help="The local host port for live web server (default: %(default)s).")

def serve(config_file=None, host=None, port=None, num_threads=None, template=None):
    """
    Create live server
    """

    # Location of serve site
    tempdir = os.path.abspath(os.path.join(os.getenv('HOME'), '.local', 'share', 'moose', 'site'))

    # Clean the "temp" directory (if desired)
    if os.path.exists(tempdir):
        log.info('Cleaning build directory: {}'.format(tempdir))
        shutil.rmtree(tempdir)

    # Create the "temp" directory
    if not os.path.exists(tempdir):
        os.makedirs(tempdir)

    # Perform the initial build
    log.info("Building documentation...")

    # Wrapper for building complete website
    def build_complete():
        return build.build_site(config_file=config_file, site_dir=tempdir, num_threads=num_threads, template=template)
    config, parser, builder = build_complete()

    # Create the live server
    server = livereload.Server()

    # Watch markdown files
    for page in builder:
        server.watch(page.source(), page.build)

    # Watch support directories
    server.watch(os.path.join(os.getcwd(), 'media'), builder.copyFiles)
    server.watch(os.path.join(os.getcwd(), 'css'), builder.copyFiles)
    server.watch(os.path.join(os.getcwd(), 'js'), builder.copyFiles)
    server.watch(os.path.join(os.getcwd(), 'fonts'), builder.copyFiles)

    # Watch the files and directories that require complete rebuild
    moose_extension = MooseDocs.get_moose_markdown_extension(parser)
    if moose_extension:
        server.watch(os.path.join(os.getcwd(), moose_extension.getConfig('executable')), build_complete)
    server.watch(config_file, build_complete)
    server.watch(config['navigation'], builder.build)
    server.watch('templates', builder.build)

    # Start the server
    server.serve(root=tempdir, host=host, port=port, restart_delay=0)
