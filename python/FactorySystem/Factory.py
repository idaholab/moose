# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import json
import inspect
from types import ModuleType
from typing import Optional
import importlib.util
import glob


class Factory:
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
        if classes is not None:
            for aclass in classes:
                classes.extend(self.getClassHierarchy(aclass.__subclasses__()))
        return classes

    def loadPlugins(self, base_dirs, plugin_path, attribute):
        for dir in base_dirs:
            dir = os.path.join(dir, plugin_path)
            if not os.path.isdir(dir):
                continue

            for file in glob.glob(os.path.join(dir, "*.py")):
                if file.endswith("__init__.py"):
                    continue

                module = self._loadModuleFromPath(file)
                if module is None:
                    raise ImportError(f"Could not import {file}")
                    continue

                for name, obj in inspect.getmembers(module, inspect.isclass):
                    # Ensure class is defined in this exact module
                    if getattr(obj, "__module__", None) != module.__name__:
                        continue
                    # Make sure it has the requested attribute
                    at = getattr(obj, attribute, None)
                    if isinstance(at, bool) and at:
                        self.register(obj, name)

    import pathlib
    @staticmethod
    def _loadModuleFromPath(py_path: pathlib.Path) -> Optional[ModuleType]:
        """
        Import a Python module from a file path without requiring it to be on sys.path.
        Returns the loaded module object.
        """
        # Create a unique-but-readable module name to avoid collisions
        unique_suffix = hex(abs(hash(os.path.abspath(py_path))) & 0xFFFFFFFF)[2:]
        mod_name = os.path.basename(py_path).split(".")[0]
        mod_name = f"_autoload_{mod_name}_{unique_suffix}"

        spec = importlib.util.spec_from_file_location(mod_name, py_path)
        if spec is None or spec.loader is None:
            return None
        module = importlib.util.module_from_spec(spec)
        # Register before exec so intra-module relative imports (rare here) can work
        sys.modules[mod_name] = module
        spec.loader.exec_module(module)
        return module

    def printDump(self, root_node_name):
        print("[" + root_node_name + "]")

        for name, object in sorted(self.objects.items()):
            print("  [./" + name + "]")

            params = self.validParams(name)

            for key in sorted(params.desc):
                default = ""
                if params.isValid(key):
                    the_param = params[key]
                    if type(the_param) is list:
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
