#!/usr/bin/env python
from peacock.utils import ExeLauncher
import json
import mooseutils

class JsonData(object):
    """
    Class that holds the json produced by an executable.
    """

    def __init__(self, app_path="", **kwds):
        """
        Constructor.
        Input:
            app_path: Path to the executable.
        """
        super(JsonData, self).__init__(**kwds)

        self.json_data = None
        self.app_path = None
        if app_path:
            self.appChanged(app_path)

    def appChanged(self, app_path):
        """
        Called when the executable changed.
        Input:
            app_path: New executable path
        """
        try:
            raw_data = self._getRawDump(app_path)
            self.json_data = json.loads(raw_data)
            self.app_path = app_path
        except Exception as e:
            mooseutils.mooseWarning("Failed to load json from '%s': %s" % (app_path, e))

    def _getRawDump(self, app_path):
        """
        Generate the raw data from the executable.
        Return:
            the data
        """
        data = ExeLauncher.runExe(app_path, "--json")
        data = data.split('**START JSON DATA**\n')[1]
        data = data.split('**END JSON DATA**')[0]
        return data

    def toPickle(self):
        """
        Return a dict that can be pickled
        """
        return {"app_path": self.app_path,
                "json_data": self.json_data,
                }

    def fromPickle(self, data):
        """
        Read in from a dict that was once pickled.
        Input:
            data[dict]: dict that was generated from toPickle()
        """
        self.app_path = data["app_path"]
        self.json_data = data["json_data"]

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: %s <exe path>" % sys.argv[0])
        sys.exit(1)
    j = JsonData(sys.argv[1])
    print(j.json_data.keys())
