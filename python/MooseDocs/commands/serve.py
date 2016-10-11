import os
import sys
import glob
import re
import mkdocs
import livereload
import logging
import shutil
import MooseDocs
import build
log = logging.getLogger(__name__)


def serve_options(parser, subparser):
    """
    Command-line options for serve command.
    """

    serve_parser = subparser.add_parser('serve', help='Generate and Sever the documentation using a local server.')
    serve_parser.add_argument('--livereload', dest='livereload', action='store_const', const='livereload', help="Enable the live reloading server.")
    serve_parser.add_argument('--dirtyreload', dest='livereload', action='store_const', const='dirtyreload', help="Enable the live reloading server without rebuilding entire site with single file change (default).")
    serve_parser.add_argument('--no-livereload', dest='livereload', action='store_const', const='no-livereload', help="Disable the live reloading of the served site.")
    serve_parser.add_argument('--strict', action='store_true', help='Enable strict mode and abort on warnings.')
    serve_parser.add_argument('--dirty', action='store_false', dest='clean', help='Do not clean the temporary build prior to building site.')
    return serve_parser

def serve(config_file='mkdocs.yml', strict=None, livereload='dirtyreload', clean=True, pages='pages.yml', page_keys=[], **kwargs):
    """
    Mimics mkdocs serve command.
    """

    # Location of serve site
    tempdir = os.path.abspath(os.path.join(os.getenv('HOME'), '.local', 'share', 'moose', 'site'))

    # Clean the "temp" directory (if desired)
    if clean and os.path.exists(tempdir):
        log.info('Cleaning build directory: {}'.format(tempdir))
        shutil.rmtree(tempdir)

    # Create the "temp" directory
    if not os.path.exists(tempdir):
        os.makedirs(tempdir)

    def builder(**kwargs):
        clean = kwargs.pop('clean', livereload != 'dirtyreload')
        live_server = livereload in ['dirtyreload', 'livereload']
        config = build.build(live_server=live_server, site_dir=tempdir,  pages=pages, page_keys=page_keys, clean_site_dir=clean, **kwargs)
        return config

    # Perform the initial build
    log.info("Building documentation...")
    config = builder(clean=clean, **kwargs)
    host, port = config['dev_addr'].split(':', 1)

    try:
        if livereload in ['livereload', 'dirtyreload']:
            mkdocs.commands.serve._livereload(host, port, config, builder, tempdir)
        else:
            mkdocs.commands.serve._static_server(host, port, tempdir)
    finally:
        log.info("Finished serving local site.")
