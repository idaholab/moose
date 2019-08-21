#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Tools for extracting C++ class information."""
import os
import re

import mooseutils

import MooseDocs

#: Locates class definitions in header files
DEFINITION_RE = re.compile(r'class\s*(?P<class>\w+)\b[^;]')

#: Locates class inhertence
CHILD_RE = re.compile(r'\bpublic\s+(?P<key>\w+)\b')

#: Locates class use in input files
INPUT_RE = re.compile(r'\btype\s*=\s*(?P<key>\w+)\b')

class DatabaseItem(object):
    """Storage container for class information."""
    def __init__(self, name, header, source):
        self.name = name
        self.source = source
        self.header = header
        self.inputs = set()
        self.children = set()

def build_class_database(include_dirs=None, input_dirs=None):
    """
    Create the class database.

    Returns a dict() of DatabaseItem objects. The key is the class name e.g., Diffusion.

    Inputs:
        include_dirs[list]: A space separated str or a list of include directories.
        input_dirs[list]: A space separated str or a list of input file directories.
    """

    # Handle environment variables
    if include_dirs:
        include_dirs = [mooseutils.eval_path(x) for x in include_dirs]
    else:
        include_dirs = [MooseDocs.ROOT_DIR]

    if input_dirs:
        input_dirs = [mooseutils.eval_path(x) for x in input_dirs]
    else:
        input_dirs = [MooseDocs.ROOT_DIR]

    # Locate filenames
    headers = _locate_filenames(include_dirs, '.h')
    inputs = _locate_filenames(input_dirs, '.i')

    # Create the database
    objects = dict()
    _process(objects, headers, DEFINITION_RE, _match_definition)
    _process(objects, headers, CHILD_RE, _match_child)
    _process(objects, inputs, INPUT_RE, _match_input)
    return objects

def _locate_filenames(directories, ext):
    """Locate files in the directories with the given extension."""

    out = set()
    for location in directories:
        for filename in mooseutils.git_ls_files(os.path.join(MooseDocs.ROOT_DIR, location)):
            if filename.endswith(ext) and not os.path.islink(filename):
                out.add(filename)
    return out

def _process(objects, filenames, regex, func):
    """Process regex"""
    for filename in filenames:
        with open(filename, 'r', encoding='utf-8') as fid:
            content = fid.read()

        for match in regex.finditer(content):
            func(objects, filename, match)

def _match_definition(objects, filename, match):
    """Class definition match function."""
    name = match.group('class')
    src = filename.replace('/include/', '/src/')[:-2] + '.C'
    if not os.path.exists(src):
        src = None
    else:
        src = os.path.relpath(src, MooseDocs.ROOT_DIR)

    hdr = os.path.relpath(filename, MooseDocs.ROOT_DIR)
    objects[name] = DatabaseItem(name, hdr, src)

def _match_child(objects, filename, match):
    """Child class match function."""
    key = match.group('key')
    if key in objects:
        filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)
        objects[key].children.add(filename)

def _match_input(objects, filename, match):
    """Input use match function."""
    key = match.group('key')
    if key in objects:
        filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)
        objects[key].inputs.add(filename)
