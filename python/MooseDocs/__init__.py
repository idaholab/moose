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
import sys
import re
import argparse
import subprocess
import multiprocessing
import collections
import logging

from MooseMarkdown import MooseMarkdown

import mooseutils

# Check for the necessary packages, this does a load so they should all get loaded.
if mooseutils.check_configuration(['yaml', 'jinja2', 'markdown', 'mdx_math', 'bs4', 'lxml',
                                   'pylatexenc']):
    sys.exit(1)

import yaml #pylint: disable=wrong-import-position

MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), '..', 'moose'))
if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.path.join(os.getenv('HOME'), 'projects', 'moose')

ROOT_DIR = subprocess.check_output(['git', 'rev-parse', '--show-toplevel'],
                                   cwd=os.getcwd(),
                                   stderr=subprocess.STDOUT).strip('\n')

TEMP_DIR = os.path.abspath(os.path.join(os.getenv('HOME'), '.local', 'share', 'moose'))

def abspath(*args):
    """
    Create an absolute path from paths that are given relative to the ROOT_DIR.

    Inputs:
      *args: Path(s) defined relative to the git repository root directory as defined in ROOT_DIR.
    """
    return os.path.abspath(os.path.join(ROOT_DIR, *args))


def relpath(abs_path):
    """
    Create a relative path from the absolute path given relative to the ROOT_DIR.

    Inputs:
      abs_path[str]: Absolute path that to be converted to a relative path to the git repository
                     root directory as defined in ROOT_DIR
    """
    return os.path.relpath(abs_path, ROOT_DIR)

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
        filename = abspath(self.construct_scalar(node))
        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)
        else:
            raise IOError("Unknown included file: {}".format(filename))


def yaml_load(filename):
    """
    Load a YAML file capable of including other YAML files.

    Args:
      filename[str]: The name to the file to load, relative to the git root directory
      loader[yaml.Loader]: The loader to utilize.
    """

    # Attach the include constructor to our custom loader.
    Loader.add_constructor('!include', Loader.include)

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
                value[k] = v
    return out
