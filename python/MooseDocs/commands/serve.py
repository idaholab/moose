import os
import sys
import glob
import re
import mkdocs
import livereload# import Server
import logging
import shutil
log = logging.getLogger(__name__)


def touch(fname, times=None):
    """
    A touch command to trigger reloading parent of nested files.

    http://stackoverflow.com/questions/1158076/implement-touch-using-python
    """
    with open(fname, 'a'):
        os.utime(fname, times)

class MooseDocsWatcher(livereload.watcher.Watcher):
    """
    Custom watcher for recursive directory watching and nested markdown files.
    """

    def is_glob_changed(self, path, ignore=None):
        """
        Implement a recurseive glob, which is only available in python 3.5.
        """

        if sys.version < (3, 5) and '**' in path:
            start, wildcard = path.split('**', 1)
            for root, dirnames, filenames in os.walk(start):
                for name in dirnames:
                    for f in glob.glob(os.path.join(root, name, wildcard.strip(os.path.sep))):
                        if self.is_file_changed(f, ignore):
                            return True
                return False
            else:
                return super(MkDocsWatcher, self).is_glob_changed(path, ignore)

    def is_file_changed(self, path, ignore=None):
        """
        Implements examinining nested files.
        """
        with open(path) as fid:
            content = fid.read()
            for match in re.finditer(r'\{!(.*?)!\}', content):
                subpage = match.group(1)
                if os.path.exists(subpage):
                    if self.is_file_changed(subpage, ignore):
                        touch(path)
                        return True
        return super(MooseDocsWatcher, self).is_file_changed(path, ignore)


def _livereload(host, port, config, builder, site_dir):
    """
    Mimics the mkdocs.commands.serve._livereload function.

    @TODO: When the mkdocs plugin system allows for custom Watcher this should be removed.
    """

    # We are importing here for anyone that has issues with livereload. Even if
    # this fails, the --no-livereload alternative should still work.
    from livereload import Server

    watcher = MooseDocsWatcher()
    server = livereload.Server(None, watcher)

    # Watch the documentation files, the config file and the theme files.
    server.watch(config['docs_dir'], builder)
    server.watch(config['config_file_path'], builder)

    for d in config['theme_dir']:
        server.watch(d, builder)

    server.serve(root=site_dir, host=host, port=int(port), restart_delay=0)


def serve(config_file='mkdocs.yml', strict=None, livereload='dirtyreload', clean=True):
    """
    Mimics mkdocs serve command.

    @TODO: When the mkdocs plugin system allows for custom Watcher this should be removed.
    """

    # Location of serve site
    tempdir = os.path.abspath('.moosedocs')

    # Clean the "temp" directory (if desired)
    if clean and os.path.exists(tempdir):
        log.info('Cleaning build directory: {}'.format(tempdir))
        shutil.rmtree(tempdir)

    # Create the "temp" directory
    if not os.path.exists(tempdir):
        os.mkdir(tempdir)

    def builder(**kwargs):
        config = mkdocs.config.load_config(config_file=config_file, strict=strict)
        config['site_dir'] = tempdir
        live_server = livereload in ['dirtyreload', 'livereload']
        dirty = kwargs.pop('dirty', livereload == 'dirtyreload')
        mkdocs.commands.build.build(config, live_server=live_server, dirty=dirty)
        return config

    # Perform the initial build
    log.info("Building documentation...")
    config = builder(dirty=not clean)
    host, port = config['dev_addr'].split(':', 1)

    try:
        if livereload in ['livereload', 'dirtyreload']:
            _livereload(host, port, config, builder, tempdir)
        else:
            mkdocs.commands.serve._static_server(host, port, tempdir)
    finally:
        log.info("Finished serving local site.")
