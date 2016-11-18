import os
import re
import yaml
import logging
log = logging.getLogger(__name__)
import MooseDocs

class PagesHelper(object):
  """
  A helper class for checking if a markdown file is include in the 'pages.yml' file.
  """
  def __init__(self, pages):
    self.pages = MooseDocs.yaml_load(pages)
    self.raw = yaml.dump(self.pages, default_flow_style=False)

  def check(self, filename):
    return filename in self.raw

  @staticmethod
  def create(root):
    """
    Generated nested 'pages.yml' files.
    """

    # Define the pages.yml file
    pages = os.path.join(root, 'pages.yml')
    content = []
    if not os.path.exists(root):
      os.makedirs(root)

    # Loop through the contents of the directory
    for item in os.listdir(root):
      full = os.path.join(root, item)
      txt = None
      if os.path.isdir(full):
        txt = '- {}: !include {}\n'.format(item, os.path.join(full, 'pages.yml'))
        PagesHelper.create(full)
      elif full.endswith('.md'):
        txt = '- {}: {}\n'.format(item[:-3], full)

      if txt:
        if txt.startswith('- Overview:'):
          content.insert(0, txt)
        else:
          content.append(txt)

    # Write the contents
    with open(pages, 'w') as fid:
      log.info('Writing pages file: {}'.format(pages))
      for line in content:
        fid.write(line)


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
    root[str]: The root directory.

  Optional Args:
    paths[list]: Valid source directory to extract syntax.
    doxygen[str]: The URL to the doxygen page.
    doxygen_name_style[str]: 'upper' (classMyClassName) and 'lower' (class_my_class_name) Doxygen html class format switch.
    pages[list]: The .yml file containing the website layout
    name[str]: The name of the syntax group (i.e., the key used in the 'locations' configuration for MooseMarkdown)
    install[str]: The install directory for the markdown (see MooseMarkdown config)
  """

  def __init__(self, yaml_data, paths=[], doxygen=None, doxygen_name_style='upper', pages='pages.yml', name=None, install=None, stubs=False, pages_stubs=False, hide=[], **kwargs):

    # Store the input variables
    self._yaml_data = yaml_data
    self.paths = paths
    self.install = install
    self.stubs = stubs
    self.pages_stubs = pages_stubs
    self.name = name
    self.hide = hide

    if pages:
      self.pages = PagesHelper(pages)

    # The databases containing the system/object/markdown/source information for this directory
    self._systems = set()
    self._objects = dict()
    self._filenames = dict()
    self._syntax = set()
    self._markdown = list() # A list of markdown files, used for updating pages.yml
    self._doxygen = doxygen
    self._doxygen_name_style = doxygen_name_style

    # Update the syntax maps
    for path in paths:
      full_path = MooseDocs.abspath(path)
      if not os.path.exists(full_path):
        log.critical("Unknown source directory supplied: {}".format(full_path))
        raise IOError(full_path)
      self._updateSyntax(path)

    for s in self._syntax:
      nodes = self._yaml_data[s]
      for node in nodes:
        name = node['name'].split('/')[-1]
        if name not in self._objects:
          self._systems.add(node['name'].rstrip('/*'))
        else:
          name = node['name'].rsplit('/', 1)[0]
          self._systems.add(name)

  def doxygen(self, name):
    """
    Returns the complete Doxygen website path for the supplied C++ object name.
    """
    if self._doxygen_name_style == 'lower':
      convert = lambda str: re.sub('(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))', '_\\1', str).lower().strip('_')
      return os.path.join(self._doxygen, "class_{}.html".format(convert(name)))
    else:
      return os.path.join(self._doxygen, "class{}.html".format(name))


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
    if name not in self._objects:
      log.error("The supplied syntax {} is not listed as a registered object, the source location where the function is registered was likely not included in the 'locations' items in the configuration file.".format(name))
      return None
    elif self._objects[name] not in self._filenames:
      log.error("Unable to locate {} in the list of files, the include location where the class is declared was likely not included in the 'locations' items in the configuration file.".format(self._objects[name]))
      return None

    return self._filenames[self._objects[name]]

  def check(self):
    """
    Check that the application documentation exists, create stubs if it does not.
    """
    for node in self._yaml_data.get():
      self._checkNode(node)

    if self.pages_stubs:
      self.pages.create(self.install)

    for md in self._markdown:
      if not self.pages.check(md):
        log.error('The markdown file {} was not found in the pages.yml'.format(md))

  def hidden(self, name):
    """
    Return True if the syntax is hidden.
    """
    return any([name.startswith(h) for h in self.hide])

  def _checkNode(self, node):
    """
    Check a YAML node.

    Args:
      node[str]: The syntax connected to this object.
    """

    full_name = node['name']
    obj_name = node['name'].split('/')[-1]
    if full_name.endswith('*') or full_name.endswith('<type>'):
      return

    if self.hasSystem(full_name):
      self._checkSystem(node)

    if self.hasObject(obj_name):
      self._checkObject(node, obj_name)

    if node['subblocks']:
      for child in node['subblocks']:
        self._checkNode(child)

  def _checkSystem(self, node):
    """
    Check the status of the documentation for a system.

    Args:
      node[str]: The syntax connected to this system.
    """
    # The full name of the object
    name = node['name']
    if self.hidden(name):
      return

    stub = '<!-- MOOSE System Documentation Stub: Remove this when content is added. -->\n'

    # Determine the filename
    if node['subblocks']:
      filename = os.path.join(self.install, name.rstrip('*').strip('/'), 'Overview.md')
    else:
      filename = os.path.join(self.install, name.rstrip('*').strip('/') + '.md')

    if not os.path.exists(filename):
      log.error("No documentation for {}. Documentation for this system should be created in: {}".format(name, os.path.abspath(filename)))

      if self.stubs:
        self._markdown.append(filename)
        stub += '\n# {} System\n'.format(name.split('/')[-1])
        stub += '!parameters {}\n\n'.format(name)

        has_subobjects = False
        has_subsystems = False
        if node['subblocks']:
          for child in node['subblocks']:
            if self.hasObject(child['name'].split('/')[-1]):
              has_subobjects = True
            if self.hasSystem(child['name']):
              has_subsystems = True

        if has_subobjects:
          stub += '!subobjects {} {}\n\n'.format(self.name, name)
        if has_subsystems:
          stub += '!subsystems {} {}\n\n'.format(self.name, name)

        # Write the stub file
        if not os.path.exists(os.path.dirname(filename)):
          os.makedirs(os.path.dirname(filename))
        with open(filename, 'w') as fid:
          log.info('Creating stub page for MOOSE system {}: {}'.format(name, filename))
          fid.write(stub)

    # If the file does exist, check that isn't just a stub
    else:
      self._markdown.append(filename)
      with open(filename, 'r') as fid:
        content = fid.read()
      if stub in content:
        log.error("MOOSE generated a stub page for {} system, but no content was added. Add documentation content to {}.".format(name, filename))

  def _checkObject(self, node, object_name):
    """
    Check the status of the documentation for a object.

    Args:
      node[str]: The syntax connected to this object.
      object_name[str]: The name of the object.
    """
    # The full name of the object
    name = node['name']
    if self.hidden(name):
      return

    stub = '<!-- MOOSE Object Documentation Stub: Remove this when content is added. -->'

    # Test for class description
    if not node['description']:
      log.error("No class description for {}. The 'addClassDescription' method should be used in the objects validParams function.".format(name))

    # Error if the filename does not exist and create a stub if desired
    filename = os.path.join(self.install, name.strip('/').replace('<type>', '') + '.md')
    if not os.path.exists(filename):
      log.error("No documentation for {}. Documentation for this object should be created in: {}".format(name, os.path.abspath(filename)))

      if self.stubs:
        self._markdown.append(filename)
        stub += '\n# {}\n'.format(object_name)
        stub += '!description {}\n\n'.format(name)
        stub += '!parameters {}\n\n'.format(name)
        stub += '!inputfiles {}\n\n'.format(name)
        stub += '!childobjects {}\n'.format(name)

        # Write the stub file
        if not os.path.exists(os.path.dirname(filename)):
          os.makedirs(os.path.dirname(filename))
        with open(filename, 'w') as fid:
          log.info('Creating stub page for MOOSE object {}: {}'.format(name, filename))
          fid.write(stub)

    # If the file does exist, check that isn't just a stub
    else:
      self._markdown.append(filename)
      with open(filename, 'r') as fid:
        content = fid.read()
      if stub in content:
        log.error("MOOSE generated a stub page for {} object, but no content was added. Add documentation content to {}.".format(name, filename))

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
