# pylint: disable=missing-docstring
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
# pylint: enable=missing-docstring

import os
import collections
import logging
import yaml
import re

from .eval_path import eval_path

LOG = logging.getLogger(__name__)


class _MissingInclude(object):
    """
    Sentinel returned by the optional include tag '!include?' when the target file does not exist.

    Consumers (e.g. MooseDocs.common.load_config) prune mapping keys and list items whose value is
    this sentinel so that optional dependencies (missing submodules) silently drop out of the
    configuration rather than raising an error.
    """


# Single shared instance used for identity comparisons (value is MISSING_INCLUDE)
MISSING_INCLUDE = _MissingInclude()


def is_missing_include(value):
    """Return True if the supplied value is the missing optional include sentinel."""
    return value is MISSING_INCLUDE


def prune_missing_includes(node):
    """
    Recursively remove any mapping keys or list items whose value is the MISSING_INCLUDE sentinel.

    Returns the pruned node. Operates in-place for dict/list containers.
    """
    if isinstance(node, dict):
        # identify sentinel-valued keys, then delete them
        for key in [k for k, v in node.items() if is_missing_include(v)]:
            del node[key]
        for value in node.values():
            prune_missing_includes(value)
    elif isinstance(node, list):
        # identify sentinel entries, then remove them
        for value in [v for v in node if is_missing_include(v)]:
            node.remove(value)
        for value in node:
            prune_missing_includes(value)
    return node


class IncludeYamlFile(object):
    """
    Object for handling including and reproducing output without include.
    """

    def __init__(self, items, root, parent, line="?", include=True, optional=False):

        self.optional = optional

        filename = eval_path(items[0])
        if not os.path.isabs(filename) and isinstance(root, str):
            filename = os.path.join(root, filename)

        if not os.path.exists(filename):
            msg = "Unknown include file '{}' on line {} of {}"
            raise IOError(msg.format(filename, line, parent))

        with open(filename, "r") as f:
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
        tag = "!include?" if data.optional else "!include"
        return dumper.represent_scalar(tag, " ".join(data.items))


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
        self.add_constructor("!include", Loader.include)
        self.add_constructor("!include?", Loader.include_optional)
        super(Loader, self).__init__(stream)

    def include(self, node):
        """
        Allow for the embedding of yaml files.
        """
        items = self.construct_scalar(node).split()
        obj = IncludeYamlFile(
            items, self._root, self._filename, node.line, self._include
        )
        return obj.content if self._include else obj

    def include_optional(self, node):
        """
        Allow for the embedding of yaml files that may not exist.

        If the referenced file is missing (e.g. an optional submodule that has not been checked
        out), the MISSING_INCLUDE sentinel is returned instead of raising. Callers are expected to
        prune sentinel values from the resulting data (see prune_missing_includes).
        """
        items = self.construct_scalar(node).split()
        try:
            obj = IncludeYamlFile(
                items, self._root, self._filename, node.line, self._include, optional=True
            )
        except IOError:
            LOG.warning(
                "Optional include '%s' on line %s of %s was not found and is being skipped.",
                " ".join(items),
                node.line,
                self._filename,
            )
            return MISSING_INCLUDE
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
    with open(filename, "r") as fid:
        yml = yaml.load(fid, lambda s: loader(s, root=root, include=include))
    return yml


def yaml_write(filename, content, indent=4):
    """
    Write YAML content to file
    """

    def make_dumper(*args, **kwargs):
        kwargs["indent"] = indent
        return Dumper(*args, **kwargs)

    # see IncludeYamlFile.representer
    document = yaml.dump(
        content, None, lambda *args, **kwargs: make_dumper(*args, **kwargs)
    )
    document = re.sub(
        r"(?P<tag>!include\??\s+)'(?P<string>.*?)'", r"\g<tag>\g<string>", document
    )

    with open(filename, "w") as fid:
        fid.write(document)
