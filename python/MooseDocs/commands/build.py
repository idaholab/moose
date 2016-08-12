import mkdocs

def build(config_file='mkdocs.yml', **kwargs):
    """
    Build the documentation using mkdocs build command.

    Args:
        config_file[str]: (Default: 'mkdocs.yml') The configure file to pass to mkdocs.
    """
    config = mkdocs.config.load_config(config_file, **kwargs)
    mkdocs.commands.build.build(config)
