#!/usr/bin/env python
from YamlData import YamlData
from ActionSyntax import ActionSyntax
from peacock.utils.FileCache import FileCache
import os
import cStringIO
from BlockInfo import BlockInfo
from ParameterInfo import ParameterInfo

class ExecutableInfo(object):
    """
    Holds the Yaml and action syntax of an executable.
    """
    SETTINGS_KEY = "ExecutableInfo"
    CACHE_VERSION = 2
    def __init__(self, **kwds):
        super(ExecutableInfo, self).__init__(**kwds)
        self.action_syntax = None
        self.yaml_data = None
        self.path = None
        self.path_map = {}

    def setPath(self, new_path):
        """
        Executable path set property.
        Will try to generate the yaml data and action syntax of the executable.
        """
        if not new_path:
            return

        fc = FileCache(self.SETTINGS_KEY, new_path, self.CACHE_VERSION)
        if fc.path == self.path:
            # If we are setting the path again, we need to make sure the executable itself hasn't changed
            if not fc.dirty:
                return

        self.yaml_data = None
        self.action_syntax = None
        self.path = None

        use_cache = os.environ.get("PEACOCK_DISABLE_EXE_CACHE", "0") != "1"
        if use_cache:
            obj = fc.read()
            if obj:
                self.fromPickle(obj)
                self.path = fc.path
                return

        syntax = ActionSyntax(fc.path)
        yaml = YamlData(fc.path)
        if syntax.app_path and yaml.app_path:
            self.yaml_data = yaml
            self.action_syntax = syntax
            self.path = fc.path
            self._createPathMap()
            fc.add(self.toPickle())

    def valid(self):
        """
        Check if this is a valid object.

        Returns:
            bool: Whether the executable has valid yaml and syntax
        """
        return self.path != None and self.yaml_data != None and self.action_syntax != None

    @staticmethod
    def clearCache():
        FileCache.clearAll(ExecutableInfo.SETTINGS_KEY)

    def toPickle(self):
        return {"yaml_data": self.yaml_data.toPickle(),
                "action_syntax": self.action_syntax.toPickle(),
                "path_map": self.path_map,
                "path": self.path,
                }

    def fromPickle(self, data):
        self.yaml_data = YamlData()
        self.yaml_data.fromPickle(data["yaml_data"])
        self.action_syntax = ActionSyntax()
        self.action_syntax.fromPickle(data["action_syntax"])
        self.path_map = data["path_map"]
        self.path = data["path"]

    def _createBasicInfo(self, parent, yaml):
        full_name = yaml["name"]
        info = BlockInfo(parent, full_name, self.action_syntax.isHardPath(full_name), yaml["description"])
        return info

    def _processChild(self, parent, yaml):
        info = self._createBasicInfo(parent, yaml)
        if yaml.get("subblocks"):
            for child in yaml["subblocks"]:
                child_info = self._processChild(info, child)
                path = child["name"]
                name = os.path.basename(path)
                if child_info.name == "*":
                    info.star_node = child_info
                    info.star = True
                elif child_info.name == "<type>":
                    info.types = child_info.types
                elif info.name == "<type>":
                    info.types[name] = child_info
                elif info.star and not child_info.hard:
                    info.star_node.types[name] = child_info
                else:
                    info.children[name] = child_info
                    info.children_list.append(name)
                self.path_map[path] = child_info

        if yaml.get("parameters"):
            for param in yaml["parameters"]:
                param_info = ParameterInfo(info, param["name"])
                param_info.setFromYaml(param)
                info.addParameter(param_info)
        return info

    def readFromFiles(self, yaml_file, syntax_file):
        yaml_data = YamlData()
        yaml_data.readFromFile(yaml_file)
        syntax = ActionSyntax()
        syntax.readFromFile(syntax_file)

        self.path = "From Files"
        self.yaml_data = yaml_data
        self.action_syntax = syntax
        self._createPathMap()

    def _createPathMap(self):
        self.path_map = {}
        self.root_info = BlockInfo(None, "/", False, "root node")
        for child in self.yaml_data.yaml_data:
            child_info = self._processChild(self.root_info, child)
            self.root_info.addChildBlock(child_info)
            self.path_map[child_info.path] = child_info
        self.path_map["/"] = self.root_info

    def _dumpNode(self, output, entry, level, prefix='  ', only_hard=False):
        if not only_hard or entry.hard:
            hard = "hard"
            if not entry.hard:
                hard = "not hard"
            star = "star"
            if not entry.star:
                star = "not star"
            output.write("%s%s: %s: %s\n" % (prefix*level, entry.path, hard, star))
            for c in entry.children_list:
                self._dumpNode(output, entry.children[c], level+1, prefix, only_hard=only_hard)

    def dumpDefaultTree(self, hard_only=False):
        output = cStringIO.StringIO()
        for c in sorted(self.path_map.keys()):
            if c == "/":
                continue
            self._dumpNode(output, self.path_map[c], 0, only_hard=hard_only)
        return output.getvalue()

if __name__ == '__main__':
    import sys
    #import json
    if len(sys.argv) < 2:
        print("Usage: <path_to_exe>")
        exit(1)
    exe_info = ExecutableInfo()
    exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    #print(json.dumps(exe_info.path_map, indent=2))
    #print(exe_info.dumpDefaultTree(False))
    #print(exe_info.dump())
    n = exe_info.path_map.get("/Variables")
    print(n.dump(sep='\t'))
