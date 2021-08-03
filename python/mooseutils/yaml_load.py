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
import collections
import yaml
import re

from .eval_path import eval_path

class IncludeYamlFile(object):
    """
    Object for handling including and reproducing output without include.
    """
    def __init__(self, items, root, parent, line='?', include=True):

        filename = eval_path(items[0])
        if not os.path.isabs(filename) and isinstance(root, str):
            filename = os.path.join(root, filename)

        if not os.path.exists(filename):
            msg = "Unknown include file '{}' on line {} of {}"
            raise IOError(msg.format(filename, line, parent))

        with open(filename, 'r') as f:
            content = yaml.load(f, lambda s: Loader(s, root, include))
            keys = items[1:] if len(items) > 1 else []
            for key in keys:
                content = content[int(key) if isinstance(content, list) else key]

        self.content = content
        self.items = items

    @staticmethod
    def representer(dumper, data):
        """NOTE: I tried to get this to output without adding quotes to the items, but I can't figure
                 out how to get that working. So, I just process the output in yaml_write.
        """
        return dumper.represent_scalar('!include', ' '.join(data.items))

class Loader(yaml.Loader):
    """
    A custom loader that handles nested includes. The nested includes should use absolute paths from
    the origin yaml file.

    http://stackoverflow.com/questions/528281/how-can-i-include-an-yaml-file-inside-another
    """
    def __init__(self, stream, root=None, include=True):
        self._filename = stream.name
        self._include = include
        self._root = root or os.path.dirname(self._filename)
        self.add_constructor('!include', Loader.include)
        super(Loader, self).__init__(stream)

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        """
        items = self.construct_scalar(node).split()
        obj = IncludeYamlFile(items, self._root, self._filename, node.line, self._include)
        return obj.content if self._include else obj

    def compose_node(self, parent, index):
         """
         Add the line number to the node.
         https://stackoverflow.com/questions/13319067/parsing-yaml-return-with-line-number
         """
         line = self.line
         node = yaml.Loader.compose_node(self, parent, index)
         node.line = line + 1
         return node

class Dumper(yaml.Dumper):
    """https://github.com/yaml/pyyaml/issues/234"""
    def __init__(self, *args, **kwargs):
        self.add_representer(IncludeYamlFile, IncludeYamlFile.representer)
        super().__init__(*args, **kwargs)

    def increase_indent(self, flow=False, *args, **kwargs):
        return super().increase_indent(flow=flow, indentless=False)

"""
Use OrderedDict for storing data.
https://stackoverflow.com/a/21048064/1088076
"""
def dict_representer(dumper, data):
    return dumper.represent_dict(data.items())

def dict_constructor(loader, node):
    return collections.OrderedDict(loader.construct_pairs(node))

_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG

yaml.add_representer(collections.OrderedDict, dict_representer)
yaml.add_constructor(_mapping_tag, dict_constructor)

def yaml_load(filename, loader=Loader, root=None, include=True):
    """
    Load a YAML file capable of including other YAML files and uses OrderedDict.

    Args:
      filename[str]: The name to the file to load, relative to the git root directory
    """
    with open(filename, 'r') as fid:
        yml = yaml.load(fid, lambda s: loader(s, root=root, include=include))
    return yml

def yaml_write(filename, content, indent=4):
    """
    Write YAML content to file
    """
    def make_dumper(*args, **kwargs):
        kwargs['indent'] = indent
        return Dumper(*args, **kwargs)

    # see IncludeYamlFile.representer
    document = yaml.dump(content, None, lambda *args, **kwargs: make_dumper(*args, **kwargs))
    document = re.sub(r"(?P<tag>!include\s+)'(?P<string>.*?)'", '\g<tag>\g<string>', document)

    with open(filename, 'w') as fid:
        fid.write(document)
