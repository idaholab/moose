import os
import shutil
import mkdocs
import MooseDocs
import logging
log = logging.getLogger(__name__)

def build_options(parser, subparser):
    """
    Command-line options for build command.
    """
    build_parser = subparser.add_parser('build', help='Generate and Build the documentation for serving.')
    return build_parser


def update_extra():
    """
    Loop through the js/css directories of MOOSE, if the file in the local build is older than the one in MOOSE
    then copy the new one from MOOSE.
    """
    for d in ['js', 'css']:
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', d)
        for root, dirs, files in os.walk(loc):
            for filename in files:
                src = os.path.join(loc, filename)
                dst = os.path.join(d, filename)
                if (not os.path.exists(dst)) or (os.path.getmtime(src) > os.path.getmtime(dst)):
                    dst_dir = os.path.dirname(dst)
                    if not os.path.exists(dst_dir):
                        log.debug('Creating {} directory.'.format(d))
                        os.makedirs(dst_dir)
                    log.debug('Copying file {} --> {}'.format(src, dst))
                    shutil.copy(src, dst)


def build(config_file='moosedocs.yml', live_server=False, pages='pages.yml', page_keys=[], clean_site_dir=False, **kwargs):
    """
    Build the documentation using mkdocs build command.

    Args:
        config_file[str]: (Default: 'mkdocs.yml') The configure file to pass to mkdocs.
    """
    pages = MooseDocs.load_pages(pages, keys=page_keys)
    config = mkdocs.config.load_config(config_file, pages=pages, **kwargs)
    update_extra()
    mkdocs.commands.build.build(config)
    mkdocs.utils.copy_media_files(config['docs_dir'], config['site_dir'])
    return config
