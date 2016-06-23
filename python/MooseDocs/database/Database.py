import os
import logging
log = logging.getLogger(__name__)

class Database(object):
    """
    A generic storage container for building a database of items. The primary purpose
    is to perform an os.walk in a directory and locate files with a given extension.
    From which an "item" is created (see items.py in this directory) which is used to
    populate the database.

    For example, this is used for building a map relating class name (e.g., Diffusion)
    to all the input files that use this object.

    Args:
       ext[str]: The file extension to consider (e.g., '.i').
       path[str | list]: The file path(s) to walk.
       itype[type DatabaseItem]: The type of item to create.

    Kwargs:
       All key, value pairs are configuration options passed to the items generated.
    """
    def __init__(self, ext, paths, itype, **kwargs):

        # Handle paths
        if isinstance(paths, str):
            paths = [paths]

        # Initialize member variables
        self._database = dict()
        self._itype = itype
        self._config = kwargs

        # Walk the directory, looking for files with the supplied extension.
        for path in paths:
            for root, dirs, files in os.walk(path, topdown=False):
                for filename in files:
                    if filename.endswith(ext):
                        self.update(os.path.join(root, filename))

    def update(self, filename):
        """
        Called when a file is located during the directory walk.

        Args:
            filename[str]: The complete filename.
        """

        # Instantiate the desired DatabaseItem type and extract the keys
        item = self._itype(filename, **self._config)
        keys = item.keys()

        # If the keys exists, update the database
        if keys:
            for key in keys:
                if key not in self._database:
                    self._database[key] = []
                self._database[key].append(item)

    def __getitem__(self, key):
        """
        Operator[] access to the database entry for the given key.

        Args:
            key[str]: The key to which database items are desired.

        Returns:
            [list]: A list of DatabaseItem objects.
        """
        return self._database[key]

    def __contains__(self, key):
        """
        'in' operator to test the key is in the database.

        Args:
            key[str]: The key to which database items are desired.

        Return:
            [bool]: True when the key is in the database.
        """
        return key in self._database
