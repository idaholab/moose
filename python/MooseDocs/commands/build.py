import os
import shutil
import mkdocs
import MooseDocs

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
                        os.makedirs(dst_dir)
                    shutil.copy(src, dst)


def build(config_file='mkdocs.yml', pages='pages.yml', **kwargs):
    """
    Build the documentation using mkdocs build command.

    Args:
        config_file[str]: (Default: 'mkdocs.yml') The configure file to pass to mkdocs.
    """
    pages = MooseDocs.yaml_load(pages)
    config = mkdocs.config.load_config(config_file, pages=pages, **kwargs)
    update_extra()
    mkdocs.commands.build.build(config)
    mkdocs.utils.copy_media_files(config['docs_dir'], config['site_dir'])
    return config
