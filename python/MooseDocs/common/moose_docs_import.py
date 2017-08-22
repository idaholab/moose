#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import os
import fnmatch
import logging
import MooseDocs
LOG = logging.getLogger(__name__)

def moose_docs_import(root_dir=None, include=None, exclude=None, base=None,
                      extensions=None):
    """
    Cretes a list of files to "include" from files, include, and/or exclude lists. All paths should
    be defined with respect to the repository base directory.

    Args:
        root_dir[str]: The directory which all other paths should be relative to.
        base[str]: The path to the base directory, this is the directory that is walked
                   to search for files that exists. It should be defined relative to the root_dir.
        include[list]: List of file/path globs to include, relative to the 'base' directory.
        exclude[list]: List of file/path glob patterns to exclude (do not include !), relative
                       to the 'base' directory.
        extension[str]: Limit the search to an extension (e.g., '.md')
    """

    # Define the include/exclude/extensions lists
    if include is None:
        include = []
    if exclude is None:
        exclude = []
    if extensions is None:
        extensions = ('')

    # Define root and base directories
    if root_dir is None:
        root_dir = MooseDocs.ROOT_DIR
    if not os.path.isabs(root_dir):
        root_dir = os.path.join(MooseDocs.ROOT_DIR, root_dir)

    # Check types
    if not isinstance(exclude, list) or any(not isinstance(x, str) for x in exclude):
        LOG.error('The "exclude" must be a list of str items.')
        return None
    if not isinstance(include, list) or any(not isinstance(x, str) for x in include):
        LOG.error('The "include" must be a list of str items.')
        return None

    # Loop through the base directory and create a set of matching filenames
    matches = set()
    for root, _, files in os.walk(os.path.join(root_dir, base)):
        filenames = [os.path.join(root, fname) for fname in files if fname.endswith(extensions)
                     and not fname.startswith('.')]
        for pattern in include:
            for filename in fnmatch.filter(filenames, os.path.join(root_dir, pattern)):
                matches.add(filename)

    # Create a remove list
    remove = set()
    for pattern in exclude:
        for filename in fnmatch.filter(matches, os.path.join(root_dir, pattern)):
            remove.add(filename)

    # Return a sorted lists of matches
    matches -= remove
    return sorted(matches)
