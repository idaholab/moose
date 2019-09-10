#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO
from peacock.utils.FileCache import FileCache
from .JsonData import JsonData
from .BlockInfo import BlockInfo
from .ParameterInfo import ParameterInfo

class ExecutableInfo(object):
    """
    Holds the Json of an executable.
    """
    SETTINGS_KEY = "ExecutableInfo"
    SETTINGS_KEY_TEST_OBJS = "ExecutableWithTestObjectsInfo"
    CACHE_VERSION = 4
    def __init__(self, **kwds):
        super(ExecutableInfo, self).__init__(**kwds)
        self.json_data = None
        self.path = None
        self.path_map = {}
        self.type_to_block_map = {}

    def setPath(self, new_path, use_test_objects=False):
        """
        Executable path set property.
        Will try to generate the json data of the executable.
        """
        if not new_path:
            return

        setting_key = self.SETTINGS_KEY
        extra_args = []
        if use_test_objects:
            setting_key = self.SETTINGS_KEY_TEST_OBJS
            extra_args = ["--allow-test-objects"]

        fc = FileCache(setting_key, new_path, self.CACHE_VERSION)
        if fc.path == self.path:
            # If we are setting the path again, we need to make sure the executable itself hasn't changed
            if not fc.dirty:
                return

        self.json_data = None
        self.path = None

        use_cache = os.environ.get("PEACOCK_DISABLE_EXE_CACHE", "0") != "1"
        if use_cache:
            obj = fc.read()
            if obj:
                self.fromPickle(obj)
                self.path = fc.path
                return

        json_data = JsonData(fc.path, extra_args)
        if json_data.app_path:
            self.json_data = json_data
            self.path = fc.path
            self._createPathMap()
            fc.add(self.toPickle())

    def valid(self):
        """
        Check if this is a valid object.

        Returns:
            bool: Whether the executable has valid json
        """
        return self.path != None and self.json_data != None

    @staticmethod
    def clearCache():
        FileCache.clearAll(ExecutableInfo.SETTINGS_KEY)
        FileCache.clearAll(ExecutableInfo.SETTINGS_KEY_TEST_OBJS)

    def toPickle(self):
        return {"json_data": self.json_data.toPickle(),
                "path_map": self.path_map,
                "path": self.path,
                "type_to_block_map": self.type_to_block_map,
                }

    def fromPickle(self, data):
        self.json_data = JsonData()
        self.json_data.fromPickle(data["json_data"])
        self.path_map = data["path_map"]
        self.path = data["path"]
        self.type_to_block_map = data["type_to_block_map"]

    def _createBasicInfo(self, parent, jdata, is_hard):
        full_name = os.path.join(parent.path, jdata["name"])
        info = BlockInfo(parent, full_name, is_hard, jdata.get("description", ""))
        return info

    def getDict(self, jdata, key):
        d = jdata.get(key, {})
        if not d:
            d = {}
        return d

    def _getCommonParameters(self, block):
        actions = block.get("actions", {})
        all_params = {}
        for name, data in actions.items():
            all_params.update(self.getDict(data, "parameters"))
        return all_params

    def _processChild(self, parent, jdata, is_hard):
        info = self._createBasicInfo(parent, jdata, is_hard)
        for name, child in self.getDict(jdata, "subblocks").items():
            child["name"] = name
            child_info = self._processChild(info, child, True & is_hard)
            info.addChildBlock(child_info)
            self.path_map[child_info.path] = child_info

        for name, child in self.getDict(jdata, "types").items():
            child["name"] = name
            child_info = self._processChild(info, child, False)
            info.types[name] = child_info

        if "star" in jdata:
            jdata["star"]["name"] = "*"
            star_node = self._processChild(info, jdata["star"], False)
            info.setStarInfo(star_node)

        for name, child in self.getDict(jdata, "subblock_types").items():
            child["name"] = name
            child_info = self._processChild(info, child, False)
            info.types[name] = child_info

        common_params = self._getCommonParameters(jdata)
        for name, param in common_params.items():
            param_info = ParameterInfo(info, name)
            param_info.setFromData(param)
            info.addParameter(param_info)

        for name, param in self.getDict(jdata, "parameters").items():
            param_info = ParameterInfo(info, name)
            param_info.setFromData(param)
            info.addParameter(param_info)

        for t in jdata.get("associated_types", []):
            self.type_to_block_map.setdefault(t, []).append(parent.path)

        return info

    def readFromFiles(self, json_file):
        json_data = JsonData()
        json_data.readFromFile(json_file)

        self.path = "From Files"
        self.json_data = json_data
        self._createPathMap()

    def _createPathMap(self):
        self.path_map = {}
        self.root_info = BlockInfo(None, "/", False, "root node")
        for name, block in self.json_data.json_data["blocks"].items():
            block["name"] = name
            block_info = self._processChild(self.root_info, block, True)
            self.root_info.addChildBlock(block_info)
            self.path_map[block_info.path] = block_info
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
        output = StringIO()
        for c in sorted(self.path_map.keys()):
            if c == "/":
                continue
            self._dumpNode(output, self.path_map[c], 0, only_hard=hard_only)
        return output.getvalue()

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Usage: <path_to_exe>")
        exit(1)
    exe_info = ExecutableInfo()
    exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    print("Keys: %s" % (sorted(exe_info.path_map.keys())))
    print(exe_info.type_to_block_map)
