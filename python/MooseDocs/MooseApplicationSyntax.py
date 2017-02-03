import os
import re
import collections
import logging
log = logging.getLogger(__name__)
import MooseDocs

class MooseInfoBase(object):
    """
    Base information for Actions and MooseObjects.
    """
    STUB_HEADER = '<!-- MOOSE Documentation Stub: Remove this when content is added. -->\n'

    def __init__(self, node, code=[], install='', group='', hidden=False, generate=False, check=True):

        # Define public parameters to be accessible
        self.syntax = node['name']
        self.parameters = node['parameters']
        self.description = node.get('description', '').strip()
        self.code = code
        self.hidden = hidden
        self.markdown = None # child class should populate this
        self.key = '/'.join([x for x in node['name'].split('/') if x not in ['<type>', '*']])
        self.name = self.key.split('/')[-1]
        self.install = install
        self._do_check = check

        # Protected
        self._node = node
        self.group = group
        self._generate = generate

    def check(self):
        """
        Check the status of the documentation for a MooseObject.
        """
        # Do nothing if the check is disabled
        if not self._do_check:
            return

        # Error if the filename does not exist and create a stub if desired
        elif not os.path.exists(self.markdown):
            log.error("No documentation for {}. Documentation for this object should be created in: {}".format(self.key, self.markdown))
            if self._generate:
                self.generate()

        # If the file does exist, check that isn't just a stub
        else:
            self._checkStub()

    def _checkStub(self):
        """
        Logs an error if a stub file exists, but content was not added.
        """
        with open(self.markdown, 'r') as fid:
            lines = fid.readlines()
        if MooseInfoBase.STUB_HEADER in lines[0]:
            log.error("A MOOSE generated stub page for {} exists, but no content was added. Add documentation content to {}.".format(self.key, self.markdown))

    def generate(self):
        pass

class MooseObjectInfo(MooseInfoBase):
    """
    Information for MooseObjects.
    """

    def __init__(self, *args, **kwargs):
        super(MooseObjectInfo, self).__init__(*args, **kwargs)

        base, name = self.key.strip('/').rsplit('/', 1)
        self.markdown = os.path.join(self.install, base, self.group, name + '.md')

    def check(self):
        super(MooseObjectInfo, self).check()

        # Test for class description
        if not self.description:
            log.error("No class description for {}. The 'addClassDescription' method should be used in the objects validParams function.".format(self.key))

    def generate(self):
        """
        Creates a stub MooseObject page.
        """

        stub = MooseInfoBase.STUB_HEADER
        stub += '\n# {}\n'.format(self.name)
        stub += '!description {}\n\n'.format(self.key)
        stub += '!parameters {}\n\n'.format(self.key)
        stub += '!inputfiles {}\n\n'.format(self.key)
        stub += '!childobjects {}\n'.format(self.key)

        # Write the stub file
        loc = os.path.dirname(self.markdown)
        if not os.path.exists(loc):
            os.makedirs(loc)
        with open(self.markdown, 'w') as fid:
            log.info('Creating stub page for MOOSE object {}: {}'.format(self.key, self.markdown))
            fid.write(stub)

class MooseActionInfo(MooseInfoBase):
    """
    Information for Moose Actions.
    """

    def __init__(self, *args, **kwargs):
        super(MooseActionInfo, self).__init__(*args, **kwargs)
        self.markdown = os.path.join(self.install, self.key.strip('/'), 'index.md')

    def generate(self):
        """
        Creates a stub system page.
        """
        stub = MooseInfoBase.STUB_HEADER
        stub += '\n# {} System\n'.format(self.name)
        stub += '!parameters {}\n\n'.format(self.key)
        stub += '!subobjects {}\n\n'.format(self.key)
        stub += '!subsystems {}\n\n'.format(self.key)

        # Write the stub file
        loc = os.path.dirname(self.markdown)
        if not os.path.exists(loc):
            os.makedirs(loc)
        with open(self.markdown, 'w') as fid:
            log.info('Creating stub page for MOOSE action {}: {}'.format(self.key, self.markdown))
            fid.write(stub)
            log.error("No documentation for {}. Documentation for this system should be created in: {}".format(self.key, self.markdown))

class MooseApplicationSyntax(object):
    """
    An object for handling the registered object and action syntax for a specific set of directories.

    A compiled MOOSE application contains all included libraries (i.e., framework, modules, and other applications), thus
    when an application is executed with --yaml in includes the complete syntax.

    To allow for documentation to be generated to only include the objects and syntax specific to an application the syntax
    defined in the application source directory must be separated from that of the entire library. This object builds maps to
    the registered objects and syntax specific to the application.

    Args:
      yaml_data[MooseYaml]: The MooseYaml object obtained by running the application with --yaml option.
      paths[list]: Valid source directory to extract syntax.
      doxygen[str]: The URL to the doxygen page.
      doxygen_name_style[str]: 'upper' (classMyClassName) and 'lower' (class_my_class_name) Doxygen html class format switch.
      group[str]: The name of the syntax group (i.e., the key used in the 'locations' configuration for MooseMarkdown)
      name[str]: The display name for the syntax group (e.g., Phase Field)
      install[str]: The install directory for the generating stub markdown files
      generate[bool]: When True the call to 'check' will generate stub markdown pages.
      hide[list]: A list of syntax to ignore for error checking and generation.
    """

    def __init__(self, yaml_data, paths=[], doxygen=None, name=None, doxygen_name_style='upper', group=None, install=None, generate=False, hide=[]):

        # Public member for syntax object name (i.e., the location name in the configuration file)
        self._name = name
        self._group = group

        self._yaml_data = yaml_data
        self._hide = hide
        install = MooseDocs.abspath(install) if install else None
        self._doxygen = doxygen
        self._doxygen_name_style = doxygen_name_style

        self._filenames = dict()
        self._objects = dict()
        self._actions = dict()

        # Update the syntax maps
        actions = collections.defaultdict(set)
        objects = dict()
        for path in paths:
            full_path = MooseDocs.abspath(path)
            if not os.path.exists(full_path):
                log.critical("Unknown source directory supplied: {}".format(full_path))
                raise IOError(full_path)
            self._updateSyntax(path, objects, actions)

        # Create MooseObjectInfo objects
        for key, value in objects.iteritems():
            for node in self._yaml_data['/' + key]:

                # Skip this node if it has subblocks, which is not possible for MooseObjects
                if node['subblocks']:
                    continue

                info = MooseObjectInfo(node,
                                       code=self._filenames[value],
                                       install=install,
                                       group=self._group,
                                       generate=generate,
                                       hidden=self.hidden(node['name']))
                self._objects[info.key] = info

        # Create MooseActionInfo objects
        for key, value in actions.iteritems():
            for node in self._yaml_data['/' + key]:
                code = []
                for a in value:
                    if a in self._filenames:
                        code += self._filenames[a]
                info = MooseActionInfo(node,
                                       code=code,
                                       install=install,
                                       group=self._group,
                                       generate=generate,
                                       hidden=self.hidden(node['name']))
                self._actions[info.key] = info

        # Create MooseActionInfo objects from the MooseObjectInfo
        # This is needed to allow for the !systems pages to be complete for apps that
        # do not also include the framework
        for obj_info in self._objects.itervalues():
            action_key = os.path.dirname(obj_info.key)
            if action_key not in self._actions:
                for node in self._yaml_data[action_key]:
                    info = MooseActionInfo(node,
                                           code=[],
                                           install=install,
                                           group=self._group,
                                           generate=generate,
                                           hidden=self.hidden(node['name']),
                                           check=False)
                    self._actions[info.key] = info

    def name(self):
        """
        Return the name of the syntax.
        """
        return self._name

    def doxygen(self, name):
        """
        Returns the complete Doxygen website path for the supplied C++ object name.
        """
        if self._doxygen_name_style == 'lower':
            convert = lambda str: re.sub('(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))', '_\\1', str).lower().strip('_')
            return os.path.join(self._doxygen, "class_{}.html".format(convert(name)))
        else:
            return os.path.join(self._doxygen, "class{}.html".format(name))

    def hasAction(self, name):
        """
        Returns True when the supplied name is a system in this object.
        """
        return name in self._actions

    def getAction(self, name):
        return self._actions[name]

    def actions(self, prefix=None, include_self=True):
        return self.__getHelper(self._actions, prefix, include_self)

    def hasObject(self, name):
        """
        Returns True when the supplied name is an object stored in the syntax object.
        """
        return name in self._objects

    def getObject(self, name):
        return self._objects[name]

    def objects(self, prefix=None, include_self=True):
        return self.__getHelper(self._objects, prefix, include_self)

    def check(self):
        """
        Check that the application documentation exists, create stubs if it does not.
        """
        for obj in self._objects.itervalues():
            if not obj.hidden:
                obj.check()
        for obj in self._actions.itervalues():
            if not obj.hidden:
                obj.check()

    def hidden(self, name):
        """
        Return True if the syntax is hidden.
        """
        return any([h in name for h in self._hide])

    @staticmethod
    def __getHelper(items, prefix=None, include_self=None):
        if prefix:
            out = []
            for item in items.itervalues():
                if (item.key == prefix and include_self) or (item.key != prefix and os.path.dirname(item.key) == prefix):
                    out.append(item)
            return out
        else:
            return items

    def _updateSyntax(self, path, objects, actions):
        """
        A helper for populating the syntax/filename/markdown databases. (private)

        Args:
          path[str]: A valid source directory to inspect.
        """

        # Walk the directory, looking for files with the supplied extension.
        for root, dirs, files in os.walk(MooseDocs.abspath(path), topdown=False):
            for filename in files:
                fullfile = os.path.join(root, filename)

                # Inspect source files
                if filename.endswith('.C') or filename.endswith('.h'):
                    fid = open(fullfile, 'r')
                    content = fid.read()
                    fid.close()

                    # Update class to source definition map
                    if filename.endswith('.h'):
                        for match in re.finditer(r'class\s*(?P<class>\w+)\b[^;]', content):
                            key = match.group('class')
                            self._filenames[key] = [fullfile]
                            src = fullfile.replace('/include/', '/src/')[:-2] + '.C'
                            if os.path.exists(src) and (src not in self._filenames[key]):
                                self._filenames[key].append(src)

                    # Map of registered objects
                    for match in re.finditer(r'(?<!\:)register(?!RecoverableData|edError)\w+?\((?P<key>\w+)\);', content):
                        key = match.group('key')
                        objects[key] = key

                    # Map of named registered objects
                    for match in re.finditer(r'registerNamed\w+?\((?P<class>\w+),\s*"(?P<key>\w+)"\);', content):
                        name = match.group('class')
                        key = match.group('key')
                        objects[key] = name

                    # Action syntax map
                    for match in re.finditer(r'registerActionSyntax\("(?P<action>\w+)"\s*,\s*"(?P<key>.*?)\"[,\);]', content):
                        key = match.group('key')
                        action = match.group('action')
                        actions[key].add(action)

        for root, dirs, files in os.walk(path, topdown=False):
            for filename in files:
                fullfile = os.path.join(root, filename)

                # Inspect source files
                name, ext = os.path.splitext(filename)
                if (ext == '.C') and (name in self._filenames):
                    self._filenames[name].append(fullfile)
