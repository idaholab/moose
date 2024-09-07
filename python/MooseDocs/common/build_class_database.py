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
import logging

import mooseutils

import MooseDocs
from .read import read

#: Locates class definitions in header files
# string starts with register(AD)MooseObject(Aliased)
# then the App name in double quotes, which must finish by App
# then either a comma and one or more spaces, or a comma, spaces, a word (for aliased registration)
# then the name of the registered class, captured in <class>, can be followed by quotes (alias)
# then the registering function closing parenthesis and the C++ semicolon
DEFINITION_RE = re.compile(r'register(|AD)MooseObject(|Aliased)\(\"\w+App\"(,\s+|,\s+\w+,\s+\")(?P<class>\w+)\"?\);')

#: Locates class inheritance
# <key> is captured as the first word after the 'public' word at the beginning of the expression
# with a space before and with some text (or not) after
CHILD_RE = re.compile(r'\b(public|protected|private)\s+(?P<key>\w+)\b')

#: Locates class use in input files
# expression starts with type, spaces or not, an equal, spaces or not
# <key> is captured as the first word after this
INPUT_RE = re.compile(r'\btype\s*=\s*(?P<key>\w+)\b')

class DatabaseItem(object):
    """Storage container for class information."""
    def __init__(self, name, header, source):
        self.name = name
        self.source = source
        self.header = header
        self.inputs = set()
        self.children = set()

def build_class_database(source_dirs=None, include_dirs=None, input_dirs=None):
    """
    Create the class database.

    Returns a dict() of DatabaseItem objects. The key is the class name e.g., Diffusion.

    Inputs:
        source_dirs[list]: A space separated str or a list of source directories.
        include_dirs[list]: A space separated str or a list of include directories.
        input_dirs[list]: A space separated str or a list of input file directories.
    """

    # Handle environment variables
    if source_dirs:
        source_dirs = [mooseutils.eval_path(x) for x in source_dirs]
    else:
        source_dirs = [MooseDocs.ROOT_DIR]

    if include_dirs:
        include_dirs = [mooseutils.eval_path(x) for x in include_dirs]
    else:
        include_dirs = [MooseDocs.ROOT_DIR]

    if input_dirs:
        input_dirs = [mooseutils.eval_path(x) for x in input_dirs]
    else:
        input_dirs = [MooseDocs.ROOT_DIR]

    # Locate filenames
    LOG = logging.getLogger('MooseDocs.common.build_class_database')
    try:
        sources = _locate_filenames(source_dirs, '.C')
    except FileNotFoundError as err:
        LOG.warning(err)
        sources = []
    try:
        headers = _locate_filenames(include_dirs, '.h')
    except FileNotFoundError as err:
        headers = []
        LOG.warning(err)
    try:
        inputs = _locate_filenames(input_dirs, '.i')
    except FileNotFoundError as err:
        inputs = []
        LOG.warning(err)

    # Create the database
    objects = dict()
    _process(objects, sources, DEFINITION_RE, _match_definition)
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
        content = read(filename)
        for match in regex.finditer(content):
            func(objects, filename, match)

def _match_definition(objects, filename, match):
    """Class definition match function."""
    name = match.group('class')

    hdr = filename.replace('/src/', '/include/')[:-2] + '.h'
    if not os.path.exists(hdr):
        hdr = None
    else:
        hdr = os.path.relpath(hdr, MooseDocs.ROOT_DIR)

    src = os.path.relpath(filename, MooseDocs.ROOT_DIR)
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

    # AD instances are caught for both the regular and AD object. This is done because:
    # - in many cases, the AD/nonAD are from the same template
    # - as they share parameters, it does not hurt to merge both documentations
    # - only one documentation file (the non AD) has been created for both classes usually
    # We avoid doing this when the classes source were separate and may have different parameters
    if key.replace('AD', '') in objects and (key not in objects or
         (objects[key.replace('AD', '')].source == objects[key].source)):
        full_filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)
        objects[key.replace('AD', '')].inputs.add(full_filename)
    if 'AD' in key and key in objects:
        full_filename = os.path.relpath(filename, MooseDocs.ROOT_DIR)
        objects[key].inputs.add(full_filename)
