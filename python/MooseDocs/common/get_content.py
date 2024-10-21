#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import re
import copy
import logging
import mooseutils

import MooseDocs
from ..tree import pages

LOG = logging.getLogger(__name__)

# File extensions to consider when building the content tree
FILE_EXT = ('.md', '.jpg', '.jpeg', '.gif', '.png', '.svg', '.webm', '.ogv', '.mp4', '.m4v', \
            '.pdf', '.css', '.js', '.bib', '.woff', '.woff2', '.html', '.ico', 'md.template', \
            'tar.gz', '.py')

def _build_regex(pattern):
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

def _find_files(filenames, pattern):
    """
    Locate files matching the given pattern.
    """
    out = set()
    if '*' in pattern:
        regex = _build_regex(pattern)
        for match in re.finditer(regex, '\n'.join(filenames), flags=re.MULTILINE):
            out.add(match.group(0))
    else:
        out.add(pattern)
    return out

def _doc_import(root_dir, content=None, optional=False):
    """
    Creates a list of files to "include" from patterns.

    Args:
        root_dir[str]: The directory which all other paths should be relative to.
        content[list]: List of file/path globs to include, relative to the 'base' directory, paths
                       beginning with '~' are excluded.
    """
    # Check root_dir
    if not os.path.isdir(root_dir):
        if not optional:
            LOG.error('The "root_dir" must be a valid directory: %s', root_dir)
        else:
            LOG.warning("Part of the content specified in 'config.yml' is not included in documentation since " +
                        root_dir + ' is not a valid directory')
        return None

    # Loop through the base directory and create a set of matching filenames
    filenames = set()
    for root, _, files in os.walk(root_dir, followlinks=True):
        for fname in files:
            full_name = os.path.join(root, fname)
            if os.path.isfile(full_name) and full_name.endswith(FILE_EXT):
                filenames.add(full_name)

    # Return the complete list if content is empty
    if content is None:
        return sorted(filenames)

    # Check content type
    if not isinstance(content, list) or any(not isinstance(x, str) for x in content):
        LOG.error('The "content" must be a list of str items.')
        return None

    # Build include/exclude lists
    include = []
    exclude = []
    for item in content:
        if item.startswith('~'):
            exclude.append(item[1:])
        else:
            include.append(item)

    # Create the complete set of files
    output = set()
    for pattern in include:
        output.update(_find_files(filenames, os.path.join(root_dir, pattern)))

    for pattern in exclude:
        output -= _find_files(output, os.path.join(root_dir, pattern))

    # Test the files exist
    for fname in copy.copy(output):
        if not os.path.isfile(fname):
            LOG.error("Unknown file provided in content (it is being removed):\n%s", fname)
            output.remove(fname)

    return sorted(output)

def create_file_page(name, filename, in_ext):
    """
    Create the correct node object for the given extension.
    """
    _, ext = os.path.splitext(filename)
    if ext in in_ext:
        return pages.Source(name, source=filename)
    else:
        return pages.File(name, source=filename)

def get_files(items, in_ext):
    """
    Get a list of files to consider given the content configuration.
    """

    filenames = []
    for value in items:
        if 'root_dir' not in value:
            LOG.error('The supplied items must be a list of dict items, each with a "root_dir" and '
                      'optionally a "content" entry.')

        # Get path from specified root_dir
        root = mooseutils.eval_path(value['root_dir'])
        if not os.path.isabs(root):
            root = os.path.join(MooseDocs.ROOT_DIR, root)

        # We default external content to be optional because we won't always have
        # all the submodules downloaded
        files = _doc_import(root, content=value.get('content', None), optional=value.get('external', False))
        if files is not None:
            for fname in files:
                filenames.append((root, fname, value.get('external', False)))

    return filenames

def get_items(options):
    """Helper for building content, see load_config.py"""
    items = []
    if isinstance(options, list):
        for value in options:
            if isinstance(value, dict):
                items.append(dict(value[list(value.keys())[0]]))
            else:
                items.append(dict(root_dir=value, content=None, external=False))
    elif isinstance(options, dict):
        for _, value in options.items():
            content = value.get('content', None)
            external = value.get('external', False)
            items.append(dict(root_dir=value['root_dir'], content=content, external=external))

    return items

def get_content(items, in_ext):
    """
    Create a tree of files for processing.

    Inputs:
        items: [list[dict(),...] A list of dict items, each dict entry must contain the 'root_dir'
                and 'content' fields that are passed to the doc_import function.
        in_ext[tuple]: Set of extensions to be converted (e.g., ('.md', )).
        out_ext[str]: The extension of rendered result (e.g., '.html').
    """
    if not isinstance(items, list) or any(not isinstance(x, dict) for x in items):
        LOG.error('The supplied items must be a list of dict items, each with a "root_dir" and '
                  'optionally a "content" entry.')
        return None

    roots = set()
    nodes = dict()
    for root, filename, external in get_files(items, in_ext):
        roots.add(root)
        key = filename.replace(root, '').strip('/')
        parts = key.split('/')

        # Create directory nodes if they don't exist
        for i in range(1, len(parts)):
            dir_key = os.path.join(*parts[:i])
            if dir_key not in nodes:
                nodes[dir_key] = pages.Directory(dir_key, external=external,
                                                 source=os.path.join(root, dir_key))

        # Create the file node, if it doesn't already exist. This enforces that the first
        # item in the supplied content lists is the page that is rendered.
        if key not in nodes:
            nodes[key] = create_file_page(key, filename, in_ext)

        nodes[key].external = external

    # Update the project files
    for root in roots:
        if mooseutils.git_is_repo(root):
            MooseDocs.PROJECT_FILES.update(mooseutils.git_ls_files(mooseutils.git_root_dir(root)))
        else:
            MooseDocs.PROJECT_FILES.update(mooseutils.list_files(root))

    return list(nodes.values())
