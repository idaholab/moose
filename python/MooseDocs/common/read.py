"""Utilities for reading files."""
import codecs
import re
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

    if filename.endswith(('.h', '.C')):
        content = re.sub(r'^//\*', '//', content, flags=re.MULTILINE|re.UNICODE)

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
        return u'cpp'
    elif ext == '.py':
        return u'python'
    return u'text'
