import os
import shutil
import mkdocs
import MooseDocs


def build(config_file='mkdocs.yml', pages='pages.yml', **kwargs):
    """
    Build the documentation using mkdocs build command.

    Args:
        config_file[str]: (Default: 'mkdocs.yml') The configure file to pass to mkdocs.
    """
    pages = MooseDocs.yaml_load(pages)
    config = mkdocs.config.load_config(config_file, pages=pages, **kwargs)
    mkdocs.commands.build.build(config)
    mkdocs.utils.copy_media_files(config['docs_dir'], config['site_dir'])
    return config
