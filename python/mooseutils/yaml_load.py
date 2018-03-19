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
import collections
import yaml

from eval_path import eval_path

class Loader(yaml.Loader):
    """
    A custom loader that handles nested includes. The nested includes should use absolute paths from
    the origin yaml file.

    http://stackoverflow.com/questions/528281/how-can-i-include-an-yaml-file-inside-another
    """
    def __init__(self, stream, root=None):
        self._filename = stream.name
        self._root = os.path.dirname(self._filename) if root is None else root
        self.add_constructor('!include', Loader.include)
        super(Loader, self).__init__(stream)

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        """
        filename = eval_path(self.construct_scalar(node))
        if not os.path.isabs(filename):
            filename = os.path.join(self._root, filename)

        if os.path.exists(filename):
            with open(filename, 'r') as f:
                return yaml.load(f, Loader)
        else:
            print node.line
            msg = "Unknown include file '{}' on line {} of {}"
            raise IOError(msg.format(filename, node.line, self._filename))

    def compose_node(self, parent, index):
         """
         Add the line number to the node.
         https://stackoverflow.com/questions/13319067/parsing-yaml-return-with-line-number
         """
         line = self.line
         node = yaml.Loader.compose_node(self, parent, index)
         node.line = line + 1
         return node

"""
Use OrderedDict for storing data.
https://stackoverflow.com/a/21048064/1088076
"""
def dict_representer(dumper, data):
    return dumper.represent_dict(data.iteritems())

def dict_constructor(loader, node):
    return collections.OrderedDict(loader.construct_pairs(node))

_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG

yaml.add_representer(collections.OrderedDict, dict_representer)
yaml.add_constructor(_mapping_tag, dict_constructor)

def yaml_load(filename, loader=Loader, root=None):
    """
    Load a YAML file capable of including other YAML files and uses OrderedDict.

    Args:
      filename[str]: The name to the file to load, relative to the git root directory
    """
    with open(filename, 'r') as fid:
        yml = yaml.load(fid, lambda s: loader(s, root=root))
    return yml
