#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtCore import QSettings, QStandardPaths
import os
try:
    import cPickle as pickle
except ImportError:
    import pickle
import uuid

class FileCache(object):
    """
    A class to help cache objects based on file changes.
    The primary use case was the need to cache Yaml data and action syntax
    from a MOOSE executable.
    This keeps the path of the executable as keys and it keeps the size
    and creation time of the executable. It then creates a cache file
    for that executable with the objects given pickled. If the executable
    changes then the old cache is deleted and the new pickle data is
    added.
    """
    VERSION = 1
    def __init__(self, settings_key, path, version=1):
        """
        Input:
            settings_key[str]: The key in QSettings
            path[str]: The path to check time and size on.
            version[int]: A version number of the stored data. If the format changes this can be bumped and current stored data will be deemed dirty.
        """
        super(FileCache, self).__init__()
        self.dirty = True
        self.path = os.path.abspath(path)
        self.settings_key = settings_key
        self.settings = QSettings()
        self.val = self.settings.value(settings_key, type=dict)
        self.path_data = self.val.get(path, {})
        self.stat = None
        self.no_exist = False
        self.data_version = version
        self._setDirty()

    def _setDirty(self):
        """
        Sets the dirty flag.
        If the path doesn't exist, or there is no cache data, it will be set as dirty
        """
        try:
            self.stat = os.stat(self.path)
        except:
            # If you can't stat it, then consider it dirty
            self.dirty = True
            self.no_exist = True
            return
        if (not self.path_data
                or self.path_data.get("cache_version") != self.VERSION
                or self.path_data.get("data_version") != self.data_version
                or self.stat.st_ctime != self.path_data.get("ctime")
                or self.stat.st_size != self.path_data.get("size")
                or not os.path.exists(self.path_data.get("pickle_path"))
                ):
            self.dirty = True
            return
        self.dirty = False

    def read(self):
        """
        Read the stored objects from the cache.
        Return:
            None if the path is not in the cache, else the pickled data
        """
        if self.dirty:
            return None

        try:
            with open(self.path_data["pickle_path"], "r") as f:
                data = pickle.load(f)
                return data
        except:
            return None

    @staticmethod
    def removeCacheFile(path):
        try:
            os.remove(path)
        except:
            pass

    def _getCacheDir(self):
        local_data_dir = QStandardPaths.standardLocations(QStandardPaths.CacheLocation)
        path = os.path.abspath(local_data_dir[0])
        try:
            # Apparently the cache location might not exist
            os.makedirs(path)
        except:
            pass
        return path

    def add(self, obj):
        """
        Add obj to the cache for path
        Input:
            obj: The data to be pickled and cached
        Return:
            False if the obj is already in the cache, else True
        """
        if not self.dirty or self.no_exist:
            # Cache is up to date, no need to add anything
            return False

        if self.path_data:
            self.removeCacheFile(self.path_data["pickle_path"])

        cache_dir = self._getCacheDir()

        filename = uuid.uuid4().hex
        full_path = os.path.join(cache_dir, filename)
        with open(full_path, "wb") as f:
            pickle.dump(obj, f, protocol=pickle.HIGHEST_PROTOCOL)
        self.path_data = {"ctime": self.stat.st_ctime,
                "size": self.stat.st_size,
                "pickle_path": full_path,
                "cache_version": self.VERSION,
                "data_version": self.data_version,
                }
        self.val[self.path] = self.path_data
        self.settings.setValue(self.settings_key, self.val)
        self.dirty = False
        return True

    @staticmethod
    def clearAll(settings_key):
        """
        Clear the cache files and the value in QSettings
        Input:
            settings_key[str]: The key in QSettings
        """
        settings = QSettings()
        val = settings.value(settings_key, type=dict)
        for key, val in val.items():
            FileCache.removeCacheFile(val["pickle_path"])
        settings.remove(settings_key)
        settings.sync()
