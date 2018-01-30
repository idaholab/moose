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
import sys
import re
import argparse
import subprocess
import multiprocessing
import collections
import logging

import mooseutils

# Check for the necessary packages, this does a load so they should all get loaded.
if mooseutils.check_configuration(['yaml', 'jinja2', 'markdown', 'pybtex',
                                   'pandas', 'livereload', 'bs4', 'lxml', 'pylatexenc', 'anytree']):
    sys.exit(1)

import yaml #pylint: disable=wrong-import-position

MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')

ROOT_DIR = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   cwd=os.getcwd(),
                                   stderr=subprocess.STDOUT).strip('\n')

TEMP_DIR = os.path.abspath(os.path.join(os.getenv('HOME'), '.local', 'share', 'moose'))

DEPRECATED_MARKDOWN = [(re.compile(r'(?P<command>^!input|!text|!clang)\s'), '!listing'),
                       (re.compile(r'(?P<command>^!figure|!image|!video)\s'), '!media'),
                       (re.compile(r'(?P<command>^!description)\s'), '!syntax description'),
                       (re.compile(r'(?P<command>^!parameters)\s'), '!syntax parameters'),
                       (re.compile(r'(?P<command>^!inputfiles)\s'), '!syntax inputs'),
                       (re.compile(r'(?P<command>^!childobjects)\s'), '!syntax children'),
                       (re.compile(r'(?P<command>^!systems)\s'), '!syntax complete'),
                       (re.compile(r'(?P<command>^!subsystems)\s'), '!syntax subsystems')]

def html_id(string):
    """
    Returns valid string for use as html id tag.
    """
    return re.sub(r'(-+)', '-', re.sub(r'[^\w]', '-', string).lower()).strip('-')

class Loader(yaml.Loader):
    """
    A custom loader that handles nested includes. The nested includes should use absolute paths
    from the origin yaml file.
    """

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        http://stackoverflow.com/questions/528281/how-can-i-include-an-yaml-file-inside-another
        """
        filename = os.path.join(ROOT_DIR, self.construct_scalar(node))
        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)
        else:
            raise IOError("Unknown included file: {}".format(filename))

    def importer(self, node, function):
        """
        Method for importing top-level entry from another file
        """
        filename, key = self.construct_scalar(node).split(' ')
        filename = os.path.join(ROOT_DIR, filename.replace('$MOOSE_DIR', MOOSE_DIR))
        if not os.path.exists(filename):
            raise IOError("Unknown import file: {}".format(filename))

        data = function(filename)
        if not isinstance(data, dict):
            raise IOError("The imported YAML data must contain a dict() at the top level.")
        if key not in data:
            raise IOError("The imported YAML data does not contain the desired key.")
        return data[key]

def yaml_load(filename):
    """
    Load a YAML file capable of including other YAML files.

    Args:
      filename[str]: The name to the file to load, relative to the git root directory
      loader[yaml.Loader]: The loader to utilize.
    """

    # Attach the include constructor to our custom loader.
    Loader.add_constructor('!include', Loader.include)
    Loader.add_constructor('!import', lambda x, y: Loader.importer(x, y, yaml_load))
    Loader.add_constructor('!import-config', lambda x, y: Loader.importer(x, y, load_config))

    if not os.path.exists(filename):
        raise IOError("The supplied configuration file was not found: {}".format(filename))

    with open(filename, 'r') as fid:
        yml = yaml.load(fid.read(), Loader)

    return yml

def load_config(config_file, **kwargs):
    """
    Read the MooseDocs configure file (e.g., website.yml)
    """
    out = collections.OrderedDict()
    config = yaml_load(config_file)
    for item in config:
        if isinstance(item, str):
            out[item] = dict()
        else:
            out[item.keys()[0]] = item.values()[0]

    for value in out.itervalues():
        for k, v in kwargs.iteritems():
            if k in value:
                if hasattr(value[k], 'update'):
                    value[k].update(v)
                else:
                    value[k] = v
    return out
