import os
import re
import copy
import collections
import logging
import MooseDocs

log = logging.getLogger(__name__)

class MooseApplicationSyntax(object):
    """
    An object for handling the registered object and action syntax for a specific set of directories.

    A compiled MOOSE application contains all included libraries (i.e., framework, modules, and other applications), thus
    when an application is executed with --yaml in includes the complete syntax.

    To allow for documentation to be generated to only include the objects and syntax specific to an application the syntax
    defined in the application source directory must be separated from that of the entire library. This object builds maps to
    the registered objects and syntax specific to the application.

    Args:
        yaml[MooseYaml]: The MooseYaml object obtained by running the application with --yaml option.
        path[str]: Valid source directory to extract syntax.
    """


    def __init__(self, yaml_data, path):


        log.info('Locating syntax for application.')

        # The databases containing the system/object/markdown/source information for this directory
        self._yaml_data = yaml_data#copy.copy(yaml_data)
        self._systems = set()
        self._objects = dict()
        self._filenames = dict()
        self._syntax = set()

        # Update the syntax maps
        self._updateSyntax(path)

        for s in self._syntax:
            nodes = self._yaml_data[s]
            for node in nodes:
                name = node['name'].split('/')[-1]
                if name not in self._objects:
                    self._systems.add(node['name'].rstrip('/*'))

        for obj in self._objects:
            nodes = self._yaml_data['/' + obj]
            for node in nodes:
                name = node['name'].rsplit('/', 1)[0]
                self._systems.add(name)


    def systems(self):
        """
        Return a set of MOOSE systems defined in the supplied directories.
        """
        return self._systems

    def hasSystem(self, name):
        """
        Returns True when the supplied name is a system in this object.
        """
        return name in self._systems

    def objects(self):
        """
        Returns a set of MOOSE objects defined in the supplied directories.
        """
        return self._objects

    def hasObject(self, name):
        """
        Returns True when the supplied name is an object stored in the syntax object.
        """
        return name in self._objects

    def filenames(self, name):
        """
        Return the filename(s), *h (and *.C) for the given object name.
        """
        return self._filenames[self._objects[name]]


    def _updateSyntax(self, path):
        """
        A helper for populating the syntax/filename/markdown databases. (private)

        Args:
            path[str]: A valid source directory to inspect.
        """

        def appendSyntax(key):
            key = '/' + key
            for node in self._yaml_data[key]:
                self._syntax.add(node['name'])

        # Walk the directory, looking for files with the supplied extension.
        for root, dirs, files in os.walk(path, topdown=False):
            for filename in files:
                fullfile = os.path.join(root, filename)

                # Inspect source files
                if filename.endswith('.C') or filename.endswith('.h'):

                    fid = open(fullfile, 'r')
                    content = fid.read()
                    fid.close()

                    # Update class to source definition map
                    if filename.endswith('.h'):
                        for match in re.finditer(r'class\s*(?P<class>\w+)', content):
                            self._filenames[match.group('class')] = [fullfile]

                    # Map of registered objects
                    for match in re.finditer(r'(?<!\:)register(?!RecoverableData|edError)\w+?\((?P<key>\w+)\);', content):
                        key = match.group('key')
                        self._objects[key] = key
                        appendSyntax(key)

                    # Map of named registered objects
                    for match in re.finditer(r'registerNamed\w+?\((?P<class>\w+),\s*"(?P<key>\w+)"\);', content):
                        name = match.group('class')
                        key = match.group('key')
                        self._objects[key] = name
                        appendSyntax(key)

                    # Action syntax map
                    for match in re.finditer(r'registerActionSyntax\("(?P<action>\w+)"\s*,\s*"(?P<key>.*?)\"[,\);]', content):
                        key = match.group('key')
                        appendSyntax(key)

        for root, dirs, files in os.walk(path, topdown=False):
            for filename in files:
                fullfile = os.path.join(root, filename)

                # Inspect source files
                name, ext = os.path.splitext(filename)
                if (ext == '.C') and (name in self._filenames):
                    self._filenames[name].append(fullfile)
