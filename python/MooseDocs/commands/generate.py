import os
import MooseDocs

def generate(config_file='moosedocs.yml', **kwargs):
    """
    Generates MOOSE system and object markdown files from the source code and detail markdown files.

    Args:
        config_file[str]: (Default: 'moosedocs.yml') The MooseMkDocs project configuration file.
    """

    # Configuration file
    if not os.path.exists(config_file):
        raise IOError("The supplied configuration file was not found: {}".format(config_file))

    # Generate the Markdown
    gen = MooseDocs.MooseApplicationDocGenerator(config_file)
    gen.generate()
