#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import os
import re
import logging
import MooseDocs
LOG = logging.getLogger(__name__)

def build_regex(pattern):
    """
    Build regex from paths with * and **.

    When defining a content file it is useful to simply use * and ** in glob like fashion for
    directory names. However, ** in glob for python 2.7 is not supported. Therefore, this function
    mimics this behavior using regexes.

    This function replaces * with a regex that will match any characters, except a slash,
    in between forward slashes or at the beginning and end of the path.

    Similarly, the ** will match any character, including a slash, in between forward slashes or
    at the beginning or end of the string.

    """
    out = pattern.replace('.', r'\.')

    # Replace ** or * that is proceeded by the beginning of the string or a forward slash and
    # that is followed by a forward or the end of the string. The replacement is a regex that will
    # mimic the glob ** and * behaviors.
    #    ** becomes (?:.*?) which matches any character
    #    * becomes (?:[^.]?) which matches any character except for the slash
    out = re.sub(r'(?!<^|/)(\*{2})(?=/|$)', r'(?:.*?)', out, flags=re.MULTILINE)
    out = re.sub(r'(?!<^|/)(\*{1})(?=/|$)', r'(?:[^/]*?)', out, flags=re.MULTILINE)

    # The overall regex for searching filenames must be limited to one line
    return r'^{}$'.format(out)

def find_files(filenames, pattern):
    """
    Locate files matching the given pattern.
    """
    out = set()
    regex = build_regex(pattern)
    for match in re.finditer(regex, '\n'.join(filenames), flags=re.MULTILINE):
        out.add(match.group(0))
    return out

def moose_docs_import(root_dir=None, include=None, exclude=None, base=None, extensions=None):
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
    filenames = set()
    for root, _, files in os.walk(os.path.join(root_dir, base)):
        for fname in files:
            if fname.endswith(extensions) and not fname.startswith('.'):
                full_name = os.path.join(root, fname)
                filenames.add(full_name)

    # Create the complete set of files
    output = set()
    for pattern in include:
        output.update(find_files(filenames, os.path.join(root_dir, pattern)))

    for pattern in exclude:
        output -= find_files(output, os.path.join(root_dir, pattern))

    return sorted(output)
