#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Utilities for reading files."""
import sys
import codecs
import os

def read(filename):
    """
    Reads file using utf-8 encoding.

    This function exists simply for convenience and not needing to remember to use "codecs" when
    reading files.

    Additionally, it handles the MOOSE headers automatically. The prism.js package syntax
    highlighting messes up with the headers, so this makes them sane.

    Inputs:
        filename[str]: The filename to open.
    """
    with codecs.open(filename, encoding='utf-8') as fid:
        content = fid.read()
    return content

def write(filename, content):
    """
    Write utf-8 file.
    """
    with codecs.open(filename, 'w', encoding='utf-8') as fid:
        fid.write(content)

def get_language(filename):
    """
    Auto detect the source code language, this is to allow for additions to be propagated to
    all MooseDocs stuff that needs language.

    Inputs:
        filename[str]: The filename to examine.
    """
    _, ext = os.path.splitext(filename)
    if ext in ['.C', '.h', '.cpp', '.hpp']:
        return 'cpp'
    elif ext == '.py':
        return 'python'
    return 'text'
