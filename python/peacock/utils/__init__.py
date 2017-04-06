import os
def getOptionFilenames(options, base, extension=None):
    """
    Return a list of filenames with correct absolute path.

    Inputs:
        options: The command-line options from argparse
        base: The base filenames to begin form (i.e., 'exodus')
        extension: (Optional) The file extension to parse from 'arguments' command-line option.
    """
    # Complete list of Exodus files to open
    filenames = getattr(options, base)
    if extension:
        for arg in options.arguments:
            if arg.endswith(extension):
                filenames.append(arg)

    # Make all paths absolute
    for i, fname in enumerate(filenames):
        if not os.path.isabs(fname):
            filenames[i] = os.path.abspath(os.path.join(options.start_dir, fname))

    return filenames
