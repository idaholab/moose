import os
import re

def getOptionFilenames(options, base, extensions=[]):
    """
    Return a list of filenames with correct absolute path.

    Inputs:
        options: The command-line options from argparse
        base: The base filenames to begin form (i.e., 'exodus')
        extension: (Optional) The file extension to parse from 'arguments' command-line option.
    """
    # Complete list of Exodus files to open
    if not isinstance(extensions, list):
        extensions = [extensions]

    filenames = getattr(options, base)
    if extensions:
        for arg in options.arguments:
            for e in extensions:
                if re.match(e, arg):
                    filenames.append(arg)

    # Make all paths absolute
    for i, fname in enumerate(filenames):
        if not os.path.isabs(fname):
            filenames[i] = os.path.abspath(os.path.join(options.start_dir, fname))

    return filenames
