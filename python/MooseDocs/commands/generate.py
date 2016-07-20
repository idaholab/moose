import os
import re
import MooseDocs

def generate(config_file='moosedocs.yml'):
    """
    Generates MOOSE system and object markdown files from the source code and detail markdown files.

    Args:
        config_file[str]: (Default: 'moosedocs.yml') The MooseMkDocs project configuration file.
    """

    # Configuration file
    if not os.path.exists(config_file):
        raise IOError("The supplied configuation file was not found: {}".format(config_file))

    # Generate the Markdown
    gen = MooseDocs.MooseApplicationDocGenerator(config_file)
    gen.generate()

    # To enable the use of easier markdown link creation (see extensions.MooseMarkdownLinkPreprocessor) the
    # relative path of the markdown file must be embeded in Markdown file.
    #
    # TODO: When (if?) the mkdocs plugin system is realized a plugin could be create to embeded this just prior to
    # conversion to html rather than hard-code it in here.
    path = os.path.join(os.getcwd(), 'content')
    for root, dirs, files in os.walk(path, topdown=False):
        for filename in files:
            if filename.endswith('.md'):

                # The absolute and relative filename for the current file
                fname = os.path.join(root, filename)
                rel_fname = os.path.relpath(fname, os.getcwd())

                # Read the filename
                with open(fname, 'r') as fid:
                    lines = fid.readlines()

                # Do nothing if the file is empty
                if len(lines) == 0:
                    continue

                # Check for existing paths
                match = re.search(r'<!--\s*(.*?)\s*-->', lines[0])
                if match:
                    if match.group(1) == rel_fname:
                        continue
                    else:
                        lines.pop(0)

                # Add the filename as html comment
                with open(fname, 'w') as fid:
                    fid.write('<!-- {} -->\n\n'.format(rel_fname))
                    fid.write(''.join(lines))
