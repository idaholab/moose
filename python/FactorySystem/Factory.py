# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import importlib.util
import inspect
import json
import os
import sys
from typing import Tuple


class Factory:

    _cache: dict[Tuple[str, type], list[Tuple[str, type]]] = {}
    """
    Cache of already loaded modules.

    Stored as (<file path>, <base class type) -> list[<class name>, <class type>].
    """

    def __init__(self):
        self.objects = {}  # The registered Objects array

    def register(self, type, name):
        self.objects[name] = type

    def validParams(self, type):
        return self.objects[type].validParams()

    def augmentParams(self, type, params):
        return self.objects[type].augmentParams(params)

    def create(self, type, *args, **kwargs):
        return self.objects[type](*args, **kwargs)

    def getClassHierarchy(self, classes):
        if classes != None:
            for aclass in classes:
                classes.extend(self.getClassHierarchy(aclass.__subclasses__()))
        return classes

    def loadPlugins(self, base_dirs: list[str], plugin_path: str, base_type: type):
        for dir in base_dirs:
            dir = os.path.join(dir, plugin_path)
            if not os.path.exists(dir):
                continue

            for file in os.listdir(dir):
                # Only check .py files, ignoring init
                if len(file) < 4 or file[-3:] != ".py" or file == "__init__.py":
                    continue

                # Full path to the file
                path = os.path.abspath(os.path.join(dir, file))
                path_split = path.split("/")
                # Attempt at guessing the module name; for example:
                # <root>/TestHarness/testers/RunApp.py -> TestHarness.testers.RunApp
                module_name = f"{path_split[-3]}.{path_split[-2]}.{path_split[-1][:-3]}"

                # Key for storing in Factory._cache
                cache_key = (module_name, base_type)
                # See if we've already loaded this file for this base type
                derived_classes = self._cache.get(cache_key)
                # Haven't, so load it now
                if derived_classes is None:
                    # Load the module if we haven't already
                    module = sys.modules.get(module_name)
                    if module is None:
                        spec = importlib.util.spec_from_file_location(module_name, path)
                        assert spec is not None and spec.loader is not None
                        module = importlib.util.module_from_spec(spec)
                        sys.modules[module_name] = module
                        try:
                            spec.loader.exec_module(module)
                        except Exception as e:
                            sys.modules.pop(module_name, None)
                            raise ImportError(f"Failed to import plugin {path}") from e

                    # Inspect classes in the module that derive from base_type
                    classes = inspect.getmembers(module, inspect.isclass)
                    derived_classes = [
                        (name, class_type)
                        for name, class_type in classes
                        if issubclass(class_type, base_type) and class_type != base_type
                    ]
                    # And store in the cache
                    self._cache[cache_key] = derived_classes

                # Register classes in the module that derive from base_type
                for name, class_type in derived_classes:
                    self.register(class_type, name)

    def printDump(self, root_node_name):
        print("[" + root_node_name + "]")

        for name, object in sorted(self.objects.items()):
            print("  [./" + name + "]")

            params = self.validParams(name)

            for key in sorted(params.desc):
                default = ""
                if params.isValid(key):
                    the_param = params[key]
                    if type(the_param) == list:
                        default = "'" + " ".join(the_param) + "'"
                    else:
                        default = str(the_param)

                print(
                    "%4s%-30s = %-30s # %s"
                    % ("", key, default, params.getDescription(key))
                )
            print("  [../]\n")
        print("[]")

    def printYaml(self, root_node_name):
        print("**START YAML DATA**")
        print("- name: /" + root_node_name)
        print("  description: !!str")
        print("  type:")
        print("  parameters:")
        print("  subblocks:")

        for name, object in self.objects.items():
            print("  - name: /" + root_node_name + "/ + name")
            print("    description:")
            print("    type:")
            print("    parameters:")

            params = self.validParams(name)
            for key in params.valid:
                required = "No"
                if params.isRequired(key):
                    required = "Yes"
                default = ""
                if params.isValid(key):
                    default = str(params[key])

                print("    - name: " + key)
                print("      required: " + required)
                print("      default: !!str " + default)
                print("      description: |")
                print("        " + params.getDescription(key))

        print("**END YAML DATA**")

    def printJSON(self, root_node_name):
        print("**START JSON DATA**")

        syntax = {
            "blocks": {
                root_node_name: {
                    "star": {
                        "subblock_types": {
                            name: {
                                "name": name,
                                "parameters": {
                                    key: {
                                        "name": key,
                                        "required": self.validParams(name).isRequired(
                                            key
                                        ),
                                        "default": (
                                            self.validParams(name).valid[key]
                                            if self.validParams(name).isValid(key)
                                            else ""
                                        ),
                                        "type": self.validParams(name).basicType(key),
                                        "description": self.validParams(
                                            name
                                        ).getDescription(key),
                                    }
                                    for key, value in self.validParams(
                                        name
                                    ).desc.items()
                                },
                            }
                            for name, object in self.objects.items()
                        }
                    }
                }
            },
            "global": {
                "associated_types": {"TestName": ["Tests/*"]},
                "parameters": [],
                "registered_apps": [],
            },
        }
        print(json.dumps(syntax, sort_keys=True, indent=4))

        print("**END JSON DATA**")
