import os.path
import re, copy
from peacock.utils import ExeLauncher
import mooseutils

class ActionSyntax(object):
    """
    Holds the output of an executable run with "--syntax"
    """

    def __init__(self, app_path=""):
        """
        Constructor.
        Input:
            app_path: path to the executable
        """
        super(ActionSyntax, self).__init__()

        self.app_path = None
        self.paths = []
        self.hard_paths = []
        self.hard_path_patterns = {}
        if app_path:
            self.appChanged(app_path)

    def appChanged(self, app_path):
        """
        The app changed.
        Input:
            app_path: path to the executable.
        """
        try:
            self.app_path = None
            data = self._getRawDump(app_path)
            self._processRawData(data)
            self.app_path = app_path
        except Exception as e:
            mooseutils.mooseWarning("Failed to load syntax from '%s': %s" % (app_path, e))

    def _getRawDump(self, app_path):
        """
        Just get the output of "--syntax"
        """
        data = ExeLauncher.runExe(app_path, "--syntax")
        data = data.split('**START SYNTAX DATA**\n')[1]
        data = data.split('**END SYNTAX DATA**')[0]
        return data

    def _processRawData(self, data):
        """
        Process the raw output of "--syntax"
        Input:
            data: the raw output
        """
        data_set = set(data.split('\n'))

        set_copy = copy.deepcopy(data_set)

        for item in set_copy:
            self.recursiveAddAllParents(data_set, item)

        self.paths = list(data_set)
        self.paths.sort()

        for path in self.paths:
            if path and path[-1] != '*':
                self.hard_paths.append(path)

        # Compile regex patterns once here so we can search them quickly later
        for hard_path in self.hard_paths:
            modified = hard_path.replace('*','[^/]*')
            modified += '$'

            p = re.compile(modified)

            self.hard_path_patterns[hard_path] = p

    def recursiveAddAllParents(self, the_set, path):
        """
        Add all parents to a path.
        Input:
            the_set: The current set of paths
            path: The path to check
        """
        if path:
            if '*' not in path:
                the_set.add(path)
            self.recursiveAddAllParents(the_set, os.path.dirname(path))

    def isHardPath(self, inpath):
        """
        Whether or not this is a hard path
        Input:
            inpath: str of path to check
        """
        path = inpath
        path = path.lstrip('/')
        for hard_path in self.hard_paths:
            if self.hard_path_patterns[hard_path].match(path):
                return True
        return False

    def getPath(self, inpath):
        """
        Get back the Action path
        If this is not a hard path it will return None
        If this path is a hard path and does not need wildcard matching it will return the same path
        In the event of wildcard matching it will return a path with stars in it
        Input:
            inpath: the path to check
        """
        path = inpath
        path = path.lstrip('/')
        for hard_path in self.hard_paths:
            mooseutils.mooseDebug("{} : {}".format(path, hard_path))
            if self.hard_path_patterns[hard_path].match(path):
                return hard_path
        return None


    def isStar(self, inpath):
        """
        Whether or not this path is a star
        Input:
            inpath: the path to check
        """
        base_path = inpath.lstrip('/')
        the_path = base_path + '/*'
        if the_path in self.paths:
            return True

        # try the case where the pattern where an
        # acceptable pattern would be <base>/*/*
        # so it should match <base>/foo
        the_path = os.path.dirname(base_path)
        the_path += '/*/*'
        return the_path in self.paths

    def inStar(self, inpath):
        """
        Whether or not this path is a star
        Input:
            inpath: the path to check
        """
        return "/*/" in inpath

    def toPickle(self):
        """
        Returns the state of this object that can be pickled
        """
        return {"app_path": self.app_path,
                "paths" : self.paths,
                "hard_paths" : self.hard_paths,
                "hard_path_patterns" : self.hard_path_patterns,
                }

    def fromPickle(self, data):
        """
        Restore state from the data as returned by toPickle().
        """
        self.app_path = data["app_path"]
        self.paths = data["paths"]
        self.hard_paths = data["hard_paths"]
        self.hard_path_patterns = data["hard_path_patterns"]
