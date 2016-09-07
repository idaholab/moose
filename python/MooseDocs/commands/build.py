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

    # Copy the css/js directories
    def ignore(src, names):
        """
        An ignore helper for the shutil.copytree command.
        """
        output = []
        for i, name in enumerate(names):
            s = os.path.join(root, name)
            d = os.path.join(src, name)
            if os.path.getmtime(s) >= os.path.getmtime(d):
                output.append(name)
        print output
        return output

    for d in ['js', 'css']:
        loc = os.path.join(MooseDocs.MOOSE_DIR, 'docs', d)
        for root, dirs, files in os.walk(loc):
            for filename in files:
                src = os.path.join(loc, filename)
                dst = os.path.join(d, filename)
                if os.path.getmtime(src) > os.path.getmtime(dst):
                    shutil.copy(src, dst)

    mkdocs.commands.build.build(config)
    mkdocs.utils.copy_media_files(config['docs_dir'], config['site_dir'])
    return config
